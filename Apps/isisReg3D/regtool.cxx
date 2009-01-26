

// C++ headers
#include <iostream>

// ITK related stuff
#include "itkImageRegistrationMethod.h"
#include "itkVersorRigid3DTransform.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkCastImageFilter.h"
#include "itkCheckerBoardImageFilter.h"
#include "itkResampleImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "metaCommand.h"

// additional stuff
#include "isisIterationObserver.h"

int main ( int argc, char** argv )
{

	const std::string progName = "isreg3d";

	/**************************************************
	 * image types
	 *************************************************/
	const unsigned int Dimension = 3;
	typedef int InputPixelType;
	typedef short OutputPixelType;
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
	typedef itk::VersorRigid3DTransform<double>
	TransformType;
	// optimizer
	typedef itk::VersorRigid3DTransformOptimizer
	OptimizerType;
	// metric
	typedef itk::MattesMutualInformationImageToImageMetric<FixedImageType,
	MovingImageType>	MetricType;
	// interpolator
	typedef itk::LinearInterpolateImageFunction<MovingImageType, double>
	InterpolatorType;

	// the registration method itself
	typedef itk::ImageRegistrationMethod<FixedImageType, MovingImageType>
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

	// checker board
	typedef itk::CheckerBoardImageFilter<FixedImageType>
	CheckerBoardFilterType;


	/**************************************************
	 * variables
	 **************************************************/
	TransformType::Pointer transform = TransformType::New();
	OptimizerType::Pointer optimizer = OptimizerType::New();
	MetricType::Pointer metric = MetricType::New();
	InterpolatorType::Pointer interpolator = InterpolatorType::New();
	RegistrationType::Pointer registration = RegistrationType::New();
	ParametersType params ( transform->GetNumberOfParameters() );

	FixedReaderType::Pointer fixedImageReader = FixedReaderType::New();
	MovingReaderType::Pointer movingImageReader = MovingReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	ResampleFilterType::Pointer resampler = ResampleFilterType::New();
	CastFilterType::Pointer caster = CastFilterType::New();
	CheckerBoardFilterType::Pointer checker = CheckerBoardFilterType::New();

	MetaCommand command;

	isis::IterationObserver::Pointer observer =
	    isis::IterationObserver::New();

	/**************************************************
	 * configuration
	 *************************************************/

	command.DisableDeprecatedWarnings();

	// -in -- the moving image file
	command.SetOption ( "input", "in", true, "The moving image." );
	command.AddOptionField ( "input","movingImageName", MetaCommand::STRING,
	                         true );

	// -out -- the output file
	command.SetOption ( "output", "out", true, "The result of the\
	                    registration process" );
	command.AddOptionField ( "output","resultImageName", MetaCommand::STRING,
	                         true );

	// -ref -- the fixed image file
	command.SetOption ( "ref", "ref", true, "The moving image." );
	command.AddOptionField ( "ref","refImageName", MetaCommand::STRING, true );

	// -iter -- number of iterations
	command.SetOption ("iter", "iter", false, "Number of iterations." );
	command.AddOptionField( "iter", "niterations", MetaCommand::INT,
							 true, "200");

	// bins - bumber of bins to compute the entropy
	command.SetOption ("bins", "bins",false,
					   "Number if bins for calculating the entropy value");
	command.AddOptionField("bins", "nbins", MetaCommand::INT, true, "30");

	// parse the commandline and quit if there are any parameter errors
	if ( !command.Parse ( argc,argv ) )
	{
		std::cout << progName << ": ERROR parsing arguments."
		<< std::endl
		<< "Usage: " << progName << "-in <moving image> -ref <fixed image>"
		<< " -out <output image>" << std::endl;
		return EXIT_FAILURE;
	}

	// configure the readers

	fixedImageReader->SetFileName ( command.GetValueAsString ( "ref",
	                                "refImageName" ) );
	movingImageReader->SetFileName ( command.GetValueAsString ( "input",
	                                 "movingImageName" ) );
	writer->SetFileName ( command.GetValueAsString ( "output",
	                      "resultImageName" ) );

	const int nbins = command.GetValueAsInt("bins", "nbins");
	const int niter = command.GetValueAsInt("iter", "niterations");

	if(niter == 0) {
	  std::cerr << progName << ": WARNING -> Number of iterations is 0."
		  << std::endl;
	}

	// preprocess the input image data

	// configure the registration method
	registration->SetMetric ( metric );
	registration->SetOptimizer ( optimizer );
	registration->SetTransform ( transform );
	registration->SetInterpolator ( interpolator );

	registration->SetFixedImage ( fixedImageReader->GetOutput() );
	registration->SetMovingImage ( movingImageReader->GetOutput() );

	// set ROI in fixed image -> maximum
	fixedImageReader->Update();
	FixedImageType::RegionType fixedImageRegion =
	    fixedImageReader->GetOutput()->GetBufferedRegion();

	// Crop the image region according to a crop factor.
	// The centers of the old and new region remain the same.
	FixedImageType::SizeType  size = fixedImageRegion.GetSize();
	FixedImageType::IndexType index = fixedImageRegion.GetIndex();

	// the cropping factor for the ROI relativ to the whole fixed image
	const float cropFactor = 0.5;
	const int dims = FixedImageType::RegionType::GetImageDimension();

	for ( int i =0;i<dims;i++ )
	{
		FixedImageType::RegionType::IndexValueType newSize =
		    static_cast<FixedImageType::RegionType::IndexValueType> (
		        size[i] * cropFactor );
		index[i] =
		    static_cast<FixedImageType::RegionType::IndexValueType> (
		        ( size[i] - newSize ) / 2.0 );
		size[i] = newSize;
	}

	fixedImageRegion.SetIndex ( index );
	fixedImageRegion.SetSize ( size );
	// end crop image region

	registration->SetFixedImageRegion ( fixedImageRegion );

	// init transform parameters
	// since we don't want to change the translational part we only initialize
	//the rotation.
	typedef TransformType::VersorType VersorType;
	typedef VersorType::VectorType VectorType;

	VersorType rotation;
	VectorType axis;

	const double angle = 0.;

	axis[0]= 0;
	axis[1]= 0;
	axis[2]= 1;

	rotation.Set ( axis, angle );

	transform->SetRotation ( rotation );

	registration->SetInitialTransformParameters ( transform->GetParameters());


	/**************************************************
	 * METRIC
	 *************************************************/
	const unsigned int nPixels = fixedImageRegion.GetNumberOfPixels();
	// be aware that this value is relativ to the CROPED image region.
	metric->SetNumberOfSpatialSamples ( static_cast<int> ( nPixels * 0.01 ) );
	//metric->SetNumberOfSpatialSamples(1000);
	// disable samples
	// metric->UseAllPixelsOn();
	metric->SetNumberOfHistogramBins ( nbins );

	/**************************************************
	 * OPTIMIZER
	 *************************************************/
	typedef OptimizerType::ScalesType ScalesType;
	ScalesType optimizerScales ( transform->GetNumberOfParameters() );
	const double translationScale = 1.0 / 1000.0;

	optimizerScales[0]=1.0;
	optimizerScales[1]=1.0;
	optimizerScales[2]=1.0;
	optimizerScales[3]=translationScale;
	optimizerScales[4]=translationScale;
	optimizerScales[5]=translationScale;

	optimizer->SetScales ( optimizerScales );
	optimizer->SetNumberOfIterations ( niter );

	// add observer
	optimizer->AddObserver ( itk::IterationEvent(), observer );

	/**************************************************
	 * start registration
	 *************************************************/
	std::cout << "Starting registration ..." << std::endl;
	try
	{
		registration->Update();
	}
	catch ( itk::ExceptionObject & err )
	{
		std::cerr << progName << ": Exception caught: " << std::endl
		<< err << std::endl;
		return EXIT_FAILURE;
	}


	/**************************************************
	 * print result
	 *************************************************/
	OptimizerType::ParametersType finalParameters =
	    registration->GetLastTransformParameters();


	const double versorX              = finalParameters[0];
	const double versorY              = finalParameters[1];
	const double versorZ              = finalParameters[2];
	const double finalTranslationX    = finalParameters[3];
	const double finalTranslationY    = finalParameters[4];
	const double finalTranslationZ    = finalParameters[5];

	const unsigned int numberOfIterations = optimizer->GetCurrentIteration();

	const double bestValue = optimizer->GetValue();


	// Print out results

	std::cout << std::endl << std::endl;
	std::cout << "Metric: Number of bins = " << nbins << std::endl;
	std::cout << "Result = " << std::endl;
	std::cout << " versor X      = " << versorX  << std::endl;
	std::cout << " versor Y      = " << versorY  << std::endl;
	std::cout << " versor Z      = " << versorZ  << std::endl;
	std::cout << " Translation X = " << finalTranslationX  << std::endl;
	std::cout << " Translation Y = " << finalTranslationY  << std::endl;
	std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
	std::cout << " Iterations    = " << numberOfIterations << std::endl;
	std::cout << " Metric value  = " << bestValue          << std::endl;

	/**************************************************
 	* resample image
	*************************************************/
	FixedImageType::Pointer fixedImage =  fixedImageReader->GetOutput();

	resampler->SetInput(movingImageReader->GetOutput());
	resampler->SetTransform(registration->GetOutput()->Get());
	resampler->SetOutputOrigin(fixedImage->GetOrigin());
	resampler->SetSize(fixedImage->GetLargestPossibleRegion().GetSize());
	resampler->SetOutputSpacing(fixedImage->GetSpacing());
	resampler->SetOutputDirection(fixedImage->GetDirection());
	resampler->SetDefaultPixelValue(0);

	// write output image

	caster->SetInput ( resampler->GetOutput() );
	writer->SetInput ( caster->GetOutput() );


	writer->Update();

	// write checker board output image

	checker->SetInput1 ( fixedImageReader->GetOutput() );
	checker->SetInput2 ( resampler->GetOutput() );
	caster->SetInput ( checker->GetOutput() );
	writer->SetFileName ( "checker.nii" );
	writer->SetInput ( caster->GetOutput() );

	writer->Update();

	// Local Variables:
	//   compile-command: "make -k -C Debug all"
	// End:
}
