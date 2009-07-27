// C++ headers
#include <iostream>

// ITK related stuff
#include "itkImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkGradientDescentOptimizer.h"
#include "itkCastImageFilter.h"
#include "itkNormalizeImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "metaCommand.h"

// additional stuff
#include "isisIterationObserver.h"

int main ( int argc, char** argv ) {

  const std::string progName = "isreg3d";

  /**************************************************
   * image types
   *************************************************/
  const unsigned int Dimension = 3;
  typedef unsigned short InputPixelType;
  typedef unsigned short OutputPixelType;
  typedef float InternalPixelType;

  // image types
  typedef itk::Image<InputPixelType, Dimension> FixedImageType;
  typedef itk::Image<InputPixelType, Dimension> MovingImageType;
  typedef itk::Image<OutputPixelType, Dimension> OutputImageType;
  typedef itk::Image<InternalPixelType, Dimension> InternalImageType;

  /**************************************************
   * IO types
   *************************************************/
  typedef itk::ImageFileReader<FixedImageType> FixedReaderType;
  typedef itk::ImageFileReader<MovingImageType> MovingReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;

  /**************************************************
   * registrator components
   **************************************************/
  // transform
  typedef itk::TranslationTransform<double,Dimension>
    TransformType;
  // optimizer
  typedef itk::GradientDescentOptimizer
    OptimizerType;
  // metric
  typedef itk::MutualInformationImageToImageMetric<InternalImageType,
    InternalImageType>	MetricType;
  // interpolator
  typedef itk::LinearInterpolateImageFunction<InternalImageType, double>
    InterpolatorType;

  // the registration method itself
  typedef itk::ImageRegistrationMethod<InternalImageType, InternalImageType>
    RegistrationType;
  typedef RegistrationType::ParametersType ParametersType;

  /**************************************************
   * additional filter
   *************************************************/
  // resampler
  typedef itk::ResampleImageFilter<MovingImageType, FixedImageType>
    ResampleFilterType;

  // caster
  typedef itk::CastImageFilter<FixedImageType, OutputImageType>
    CastFilterType;

  // normalizer
  typedef itk::NormalizeImageFilter<FixedImageType, InternalImageType>
    FixedNormalizerFilterType;
  typedef itk::NormalizeImageFilter<MovingImageType, InternalImageType>
    MovingNormalizerFilterType;
  
  // gaussian blur filter
  typedef itk::DiscreteGaussianImageFilter<InternalImageType, InternalImageType>
    GaussianFilterType;

  /**************************************************
   * variables
   **************************************************/
  TransformType::Pointer transform = TransformType::New();
  OptimizerType::Pointer optimizer = OptimizerType::New();
  MetricType::Pointer metric = MetricType::New();
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  RegistrationType::Pointer registration = RegistrationType::New();
  ParametersType params(transform->GetNumberOfParameters());

  FixedReaderType::Pointer fixedImageReader = FixedReaderType::New();
  MovingReaderType::Pointer movingImageReader = MovingReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  CastFilterType::Pointer caster = CastFilterType::New();
  MovingNormalizerFilterType::Pointer movingNormalizer = MovingNormalizerFilterType::New();
  FixedNormalizerFilterType::Pointer fixedNormalizer = FixedNormalizerFilterType::New();
  GaussianFilterType::Pointer fixedSmoother = GaussianFilterType::New();
  GaussianFilterType::Pointer movingSmoother = GaussianFilterType::New();
  MetaCommand command;

  isis::IterationObserver::Pointer observer =
    isis::IterationObserver::New();

  /**************************************************
   * configuration
   *************************************************/

  command.DisableDeprecatedWarnings();

  // -in -- the moving image file
  command.SetOption("input", "in", true, "The moving image.");
  command.AddOptionField("input","movingImageName", MetaCommand::STRING,
			 true);

  // -out -- the output file
  command.SetOption("output", "out", true, "The result of the\
 registration process");
  command.AddOptionField("output","resultImageName", MetaCommand::STRING,
			 true);

  // -ref -- the fixed image file
  command.SetOption("ref", "ref", true, "The moving image.");
  command.AddOptionField("ref","refImageName", MetaCommand::STRING, true);

  // parse the commandline and quit if there are any parameter errors
  if(!command.Parse(argc,argv)){
    std::cout << progName << ": ERROR parsing arguments."
	      << std::endl
	      << "Usage: " << progName << "-in <moving image> -ref <fixed image>"
	      << " -out <output image>" << std::endl;
    return EXIT_FAILURE;
  }
  
  // configure the readers

  fixedImageReader->SetFileName(command.GetValueAsString("ref",
						    "refImageName"));
  movingImageReader->SetFileName(command.GetValueAsString("input",
						     "movingImageName"));
  writer->SetFileName(command.GetValueAsString("output",
					       "resultImageName"));

  // preprocess the input image data
  
  // normalize
  fixedNormalizer->SetInput(fixedImageReader->GetOutput());
  movingNormalizer->SetInput(movingImageReader->GetOutput());

  // smooth
  fixedSmoother->SetInput(fixedNormalizer->GetOutput());
  movingSmoother->SetInput(movingNormalizer->GetOutput());

  // configure the registration method
  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetTransform(transform);
  registration->SetInterpolator(interpolator);

  registration->SetFixedImage(fixedSmoother->GetOutput());
  registration->SetMovingImage(movingSmoother->GetOutput());

  // set ROI in fixed image -> maximum
  fixedNormalizer->Update();
  FixedImageType::RegionType fixedImageRegion = 
    fixedNormalizer->GetOutput()->GetBufferedRegion();
  registration->SetFixedImageRegion(fixedImageRegion);

  // init transform parameters
  for (unsigned int i = 0; i < params.Size(); i++) {
    params[i] = 0;
  }
  registration->SetInitialTransformParameters(params);

  // configure the metric
  const unsigned int nPixels = fixedImageRegion.GetNumberOfPixels();
  metric->SetNumberOfSpatialSamples(static_cast<int>(nPixels * 0.001));
  metric->SetFixedImageStandardDeviation (0.4);
  metric->SetMovingImageStandardDeviation (0.4);

  // configure the optimizer

  // init optimizer parameters
  optimizer->SetLearningRate( 15 );
  optimizer->MaximizeOn();
  // maximal number of iterations
  optimizer->SetNumberOfIterations (200 );

  // add observer
  optimizer->AddObserver(itk::IterationEvent(), observer);

  /**************************************************
   * start registration
   *************************************************/
  std::cout << "Starting registration ..." << std::endl;
  try {
    registration->Update();
  }
  catch (itk::ExceptionObject & err) {
    std::cerr << progName << ": Exception caught: " << std::endl
	      << err << std::endl;
    return EXIT_FAILURE;
  }

  // get result
  params = registration->GetLastTransformParameters();
  std::cout << "translation along the X axis: " << params[0] << std::endl
	    << "translation along the Y axis: " << params[1] << std::endl
	    << "number of iterations: " << optimizer->GetCurrentIteration()
	    << std::endl << "best value: " << optimizer->GetValue()
	    << std::endl;


  // connect the resampler to the moving image and apply the
  // resulting transformation matrix
  resampler->SetInput(movingImageReader->GetOutput());
  // set transform
  resampler->SetTransform(registration->GetOutput()->Get());

  // set additional parameters from the fixed image
  FixedImageType::Pointer fixedImage =  fixedImageReader->GetOutput();
  std::cout << "FI - Origin: " << fixedImage->GetOrigin() << std::endl;
  resampler->SetOutputOrigin(fixedImage->GetOrigin());
  std::cout << "FI - Size: "
	    << fixedImage->GetLargestPossibleRegion().GetSize() << std::endl;
  resampler->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
  std::cout << "FI: Spacing: " << fixedImage->GetSpacing() << std::endl;
  resampler->SetOutputSpacing(fixedImage->GetSpacing());
  std::cout << "FI: Direction: " << std::endl
	    << fixedImage->GetDirection() << std::endl;
  resampler->SetOutputDirection(fixedImage->GetDirection());
  resampler->SetDefaultPixelValue(100);

  caster->SetInput(resampler->GetOutput());
  writer->SetInput(caster->GetOutput());


  writer->Update();


  // Local Variables:
  //   compile-command: "make -k -C Debug all"
  // End:
}
