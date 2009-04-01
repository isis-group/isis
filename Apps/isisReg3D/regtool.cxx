
// C++ headers
#include <iostream>

/* this is a test comment */

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

//masking related stuff
#include "itkImageMaskSpatialObject.h"
#include "itkImageSpatialObject.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkAndImageFilter.h"

#include "itkOtsuThresholdImageFilter.h"

int main(int argc, char** argv) {

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
  typedef itk::Image<unsigned char, Dimension> CharImageType;

  /**************************************************
   * IO types
   *************************************************/
  typedef itk::ImageFileReader<FixedImageType> FixedReaderType;
  typedef itk::ImageFileReader<MovingImageType> MovingReaderType;
  typedef itk::ImageFileWriter<OutputImageType> WriterType;
  typedef itk::ImageFileWriter<CharImageType> BinaryImageWriterType;
  /**************************************************
   * registrator components
   **************************************************/
  // transform
  typedef itk::VersorRigid3DTransform<double> TransformType;
  // optimizer
  typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
  // metric
  typedef itk::MattesMutualInformationImageToImageMetric<FixedImageType,
          MovingImageType> MetricType;
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

  typedef itk::CastImageFilter<FixedImageType, CharImageType>
    MaskCastFilterType;

  // checker board
  typedef itk::CheckerBoardImageFilter<FixedImageType> CheckerBoardFilterType;

  typedef itk::BinaryThresholdImageFilter<FixedImageType, CharImageType>
    BinaryThresholdImageFilterType;

  typedef itk::OtsuThresholdImageFilter<FixedImageType, CharImageType>
    OtsuThresholdImageFilterType;

  typedef itk::ImageMaskSpatialObject<Dimension> MaskObjectType;

  typedef itk::ImageSpatialObject<Dimension, InputPixelType>
    FixedImageSpatialObjectType;

  typedef itk::ImageSpatialObject<Dimension, InputPixelType>
    MovingImageSpatialObjectType;

  typedef itk::MinimumMaximumImageCalculator<FixedImageType>
    MinimumMaximumImageCalculatorType;

  typedef itk::AndImageFilter<CharImageType, CharImageType, CharImageType>
    AndFilterType;

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
  FixedReaderType::Pointer maskImageReader = FixedReaderType::New();
  MovingReaderType::Pointer movingImageReader = MovingReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  CastFilterType::Pointer caster = CastFilterType::New();
  MaskCastFilterType::Pointer maskCaster = MaskCastFilterType::New();
  CheckerBoardFilterType::Pointer checker = CheckerBoardFilterType::New();

  //mask related stuff
  OtsuThresholdImageFilterType::Pointer otsuThresholdFilter =
    OtsuThresholdImageFilterType::New();
  CharImageType::Pointer charImage = CharImageType::New();
  MaskObjectType::Pointer maskObject = MaskObjectType::New();
  MinimumMaximumImageCalculatorType::Pointer minMaxCalc =
    MinimumMaximumImageCalculatorType::New();
  BinaryThresholdImageFilterType::Pointer binaryThresholdFilter =
    BinaryThresholdImageFilterType::New();

  MetaCommand command;

  isis::IterationObserver::Pointer observer = isis::IterationObserver::New();

  CharImageType::Pointer jointImage = CharImageType::New();
  /**************************************************
   * configuration
   *************************************************/

  command.DisableDeprecatedWarnings();

  // -in -- the moving image file
  command.SetOption("input", "in", true, "The moving image.");
  command.AddOptionField("input", "movingImageName", MetaCommand::STRING,
      true);

  // -out -- the output file
  command.SetOption("output", "out", true,
      "The result of the registration process");
  command.AddOptionField("output", "resultImageName", MetaCommand::STRING,
      true);

  // -ref -- the fixed image file
  command.SetOption("ref", "ref", true, "The fixed image.");
  command.AddOptionField("ref", "refImageName", MetaCommand::STRING, true);

  // -iter -- number of iterations
  command.SetOption("iter", "iter", false, "Number of optimizer iterations.");
  command.AddOptionField("iter", "niterations", MetaCommand::INT, true, "200");

  // bins - bumber of bins to compute the entropy
  command.SetOption("bins", "bins", false,
      "Number of bins for calculating the entropy value");
  command.AddOptionField("bins", "nbins", MetaCommand::INT, true, "30");

  //absolute threshold
  command.SetOption("threshold_abs", "threshold_abs", false,
      "Absolute threshold for creating a mask applied during the registration");
  command.AddOptionField("threshold_abs", "vthreshold_abs", MetaCommand::INT,
      true, "0");

  //relative threshold
  command.SetOption(
      "threshold_rel",
      "threshold_rel",
      false,
      "Relative threshold for creating a mask applied during the registration. Has to be between 0 and 1");
  command.AddOptionField("threshold_rel", "vthreshold_rel",
      MetaCommand::FLOAT, true, "0");

  //option to save the mask calculated with the threshold
  command.SetOption("savemask", "sm", false, "Path to save the mask");
  command.AddOptionField("savemask", "pathsavemask", MetaCommand::STRING,
      true);

  //option to load a mask from a file
  command.SetOption("loadmask", "lm", false,
      "Loading an existing mask applied in the registration");
  command.AddOptionField("loadmask", "pathloadmask", MetaCommand::STRING,
      true);

  //option to change the crop for limiting the ROI
  command.SetOption(
      "crop",
      "crop",
      false,
      "crop for limiting the ROI. " "1" " is maximum and represents the whole image.");
  command.AddOptionField("crop", "vcrop", MetaCommand::FLOAT, true, "1");

  command.SetOption(
      "otsuMask",
      "otsuMask",
      false,
      "Using the OtsuSegmentationMethod for creating the mask. Threshold needs to be set, too.");

  command.SetOption("allSamples", "allSamples",false, "Using all samples containing to the fixedImage to calculate the metric.");


  // parse the commandline and quit if there are any parameter errors
  if (!command.Parse(argc, argv)) {
    std::cout << progName << ": ERROR parsing arguments." << std::endl
      << "Usage: " << progName
      << " -in <moving image> -ref <fixed image> [optional Options]"
      << " -out <output image>" << std::endl;
    return EXIT_FAILURE;
  }

  if ((command.GetOptionWasSet("threshold_rel") or command.GetOptionWasSet(
          "threshold_abs")) and command.GetOptionWasSet("loadmask")) {
    std::cout << "attempt to use both methods for getting the mask!"
      << std::endl;
    return EXIT_FAILURE;
  }
  if (command.GetOptionWasSet("threshold_rel") and command.GetOptionWasSet(
        "threshold_abs")) {
    std::cout
      << "attempt to use relative and absolute threshold method for getting the mask!"
      << std::endl;
    return EXIT_FAILURE;
  }
  if (command.GetOptionWasSet("threshold_rel") and command.GetValueAsFloat(
        "threshold_rel", "vthreshold_rel") > 1) {
    std::cout << "threshold_rel has to be between 0 and 1 included!"
      << std::endl;
    return EXIT_FAILURE;
  }

  // configure the readers

  fixedImageReader->SetFileName(command.GetValueAsString("ref",
        "refImageName"));
  movingImageReader->SetFileName(command.GetValueAsString("input",
        "movingImageName"));
  writer->SetFileName(command.GetValueAsString("output", "resultImageName"));

  const int nbins = command.GetValueAsInt("bins", "nbins");
  const int niter = command.GetValueAsInt("iter", "niterations");

  if (niter == 0) {
    std::cerr << progName << ": WARNING -> Number of iterations is 0."
      << std::endl;
  }

  // preprocess the input image data
  if (command.GetOptionWasSet("loadmask")) {
    maskImageReader->SetFileName(command.GetValueAsString("loadmask",
          "pathloadmask"));
    maskCaster->SetInput(maskImageReader->GetOutput());
    maskCaster->Update();
    maskObject->SetImage(maskCaster->GetOutput());

  }

  if (command.GetOptionWasSet("otsuMask")) {
    otsuThresholdFilter->SetNumberOfHistogramBins(128);
    otsuThresholdFilter->SetInput(fixedImageReader->GetOutput());
    fixedImageReader->Update();
    otsuThresholdFilter->SetInsideValue(0);
    otsuThresholdFilter->SetOutsideValue(255);
    otsuThresholdFilter->Update();
    if (command.GetOptionWasSet("savemask")) {
      std::cout << "Saving otsuMask to " << command.GetValueAsString(
          "savemask", "pathsavemask") << std::endl;
      typedef itk::ImageFileWriter<CharImageType> BinaryImageWriterType;
      BinaryImageWriterType::Pointer binaryWriter =
        BinaryImageWriterType::New();
      binaryWriter->SetFileName(command.GetValueAsString("savemask",
            "pathsavemask"));
      binaryWriter->SetInput(otsuThresholdFilter->GetOutput());
      binaryWriter->Update();
    }
    maskObject->SetImage(otsuThresholdFilter->GetOutput());
    maskObject->Update();
  }
  if (command.GetOptionWasSet("threshold_rel") or command.GetOptionWasSet(
        "threshold_abs")) {

    //FixedImageType::PixelType threshold;
    int threshold;
    if (command.GetOptionWasSet("threshold_rel")) {
      minMaxCalc->SetImage(fixedImageReader->GetOutput());
      fixedImageReader->Update();
      minMaxCalc->Compute();
      threshold = static_cast<int> (minMaxCalc->GetMaximum()
          * (command.GetValueAsFloat("threshold_rel",
              "vthreshold_rel")));
    }
    if (command.GetOptionWasSet("threshold_abs")) {
      threshold
        = command.GetValueAsInt("threshold_abs", "vthreshold_abs");
    }

    std::cout << "Threshold for creating the mask: " << threshold
      << std::endl;
    std::cout << "Maximum Image: " << minMaxCalc->GetMaximum() << std::endl;
    //used for creating the mask

    binaryThresholdFilter->SetInput(fixedImageReader->GetOutput());
    binaryThresholdFilter->SetOutsideValue(0);
    binaryThresholdFilter->SetInsideValue(255);
    binaryThresholdFilter->SetLowerThreshold(threshold);
    binaryThresholdFilter->SetUpperThreshold(minMaxCalc->GetMaximum());
    binaryThresholdFilter->Update();
    //debug
    if (command.GetOptionWasSet("savemask")) {

      BinaryImageWriterType::Pointer binaryWriter =
        BinaryImageWriterType::New();
      binaryWriter->SetFileName(command.GetValueAsString("savemask",
            "pathsavemask"));
      binaryWriter->SetInput(binaryThresholdFilter->GetOutput());
      binaryWriter->Update();
    }
    maskObject->SetImage(binaryThresholdFilter->GetOutput());
    maskObject->Update();

  }

  // configure the registration method
  registration->SetMetric(metric);
  registration->SetOptimizer(optimizer);
  registration->SetTransform(transform);
  registration->SetInterpolator(interpolator);

  registration->SetFixedImage(fixedImageReader->GetOutput());
  registration->SetMovingImage(movingImageReader->GetOutput());

  // set ROI in fixed image -> maximum
  fixedImageReader->Update();
  FixedImageType::RegionType fixedImageRegion =
    fixedImageReader->GetOutput()->GetBufferedRegion();

  // Crop the image region according to a crop factor.
  // The centers of the old and new region remain the same.
  FixedImageType::SizeType size = fixedImageRegion.GetSize();
  FixedImageType::IndexType index = fixedImageRegion.GetIndex();

  // the cropping factor for the ROI relativ to the whole fixed image
  const float cropFactor = command.GetValueAsFloat("crop", "vcrop");
  const int dims = FixedImageType::RegionType::GetImageDimension();

  for (int i = 0; i < dims; i++) {
    FixedImageType::RegionType::IndexValueType
      newSize =
      static_cast<FixedImageType::RegionType::IndexValueType> (size[i]
          * cropFactor);
    index[i]
      = static_cast<FixedImageType::RegionType::IndexValueType> ((size[i]
            - newSize) / 2.0);
    size[i] = newSize;
  }
  //fixedImageRegion.SetIndex(index);
  //fixedImageRegion.SetSize(size);

  // end crop image region


  //compare the image size of the moving image and the fixed image.
  fixedImageReader->Update();
  movingImageReader->Update();
  FixedImageType::SizeType sizeFixed =
    fixedImageReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  FixedImageType::SizeType
    sizeMoving =
    movingImageReader->GetOutput()->GetLargestPossibleRegion().GetSize();
  bool fixedImageIsBigger = false;
  for (int i = 0; i < 3; i++) {
    if (sizeFixed[i] > sizeMoving[i]) {
      fixedImageIsBigger = true;
    }
  }
  MaskObjectType::Pointer jointMask = MaskObjectType::New();
  if (fixedImageIsBigger) {

    CharImageType::Pointer fixedMaskImage = CharImageType::New();
    CharImageType::Pointer movingMaskImage = CharImageType::New();
    BinaryImageWriterType::Pointer maskWriter = BinaryImageWriterType::New();
    AndFilterType::Pointer andFilter = AndFilterType::New();
    //creating the fixed image mask
    minMaxCalc->SetImage(fixedImageReader->GetOutput());
    minMaxCalc->Compute();
    binaryThresholdFilter->SetOutsideValue(0);
    binaryThresholdFilter->SetInsideValue(255);
    binaryThresholdFilter->SetLowerThreshold( 0 );
    binaryThresholdFilter->SetUpperThreshold(minMaxCalc->GetMaximum() );
    binaryThresholdFilter->SetInput( fixedImageReader->GetOutput() );
    binaryThresholdFilter->Update();
    andFilter->SetInput1( binaryThresholdFilter->GetOutput() );
    maskWriter->SetInput( binaryThresholdFilter->GetOutput() );
    maskWriter->SetFileName("Input1.nii");
    maskWriter->Update();

    //creating moving image mask
    minMaxCalc->SetImage(movingImageReader->GetOutput());
    minMaxCalc->Compute();
    binaryThresholdFilter->SetOutsideValue(0);
    binaryThresholdFilter->SetInsideValue(255);
    binaryThresholdFilter->SetLowerThreshold( 0 );
    binaryThresholdFilter->SetUpperThreshold(minMaxCalc->GetMaximum() );
    binaryThresholdFilter->SetInput( movingImageReader->GetOutput() );
    binaryThresholdFilter->UpdateLargestPossibleRegion();
    binaryThresholdFilter->Update();
    andFilter->SetInput2( binaryThresholdFilter->GetOutput() );
    maskWriter->SetInput( binaryThresholdFilter->GetOutput() );
    maskWriter->SetFileName("Input2.nii");
    maskWriter->Update();


    //implement andfilter

    andFilter->Update();


    jointImage = andFilter->GetOutput();
    jointImage->Print(std::cout);
    maskWriter->SetInput( andFilter->GetOutput() );
    maskWriter->SetFileName("CropMask.nii");
    maskWriter->Update();
    jointMask->SetImage( andFilter->GetOutput() );
    jointMask->Update();
    jointMask->Print(std::cout);
  }
  if (fixedImageIsBigger and (!command.GetOptionWasSet("loadmask")))
  {
    //get an ImageMaskSpatialObject of the moving image size including intensity 255 over the whole volume
    //define this mask as the fixedImageMask
    std::cout << "Fixed image is bigger than moving image...creating image mask" << std::endl;
    minMaxCalc->SetImage( movingImageReader->GetOutput() );
    minMaxCalc->Compute();
    binaryThresholdFilter->SetInput( movingImageReader->GetOutput() );
    binaryThresholdFilter->SetOutsideValue( 0 );
    binaryThresholdFilter->SetInsideValue( 255 );
    binaryThresholdFilter->SetLowerThreshold( 0 );
    binaryThresholdFilter->SetUpperThreshold( minMaxCalc->GetMaximum() );
    binaryThresholdFilter->UpdateLargestPossibleRegion();
    binaryThresholdFilter->Update();
    maskObject->SetImage( binaryThresholdFilter->GetOutput() );
    maskObject->Update();

    metric->SetFixedImage( fixedImageReader->GetOutput() );
    metric->SetMovingImage( movingImageReader->GetOutput() );
    metric->SetFixedImageRegion( movingImageReader->GetOutput()->GetBufferedRegion() );
    metric->SetFixedImageMask( jointMask );
    //The option UseAllPixelOn() disables the random sampling and uses all the pixels of the FixedImageRegion in order to estimate the joint intensity PDF.
    if (command.GetOptionWasSet("allSamples"))
    {
      metric->UseAllPixelsOn();
      std::cout << "Using all pixels on" << std::endl;
    }
    if (!command.GetOptionWasSet("allSamples"))
    {
      metric->SetNumberOfSpatialSamples( static_cast<int>((movingImageReader->GetOutput()->GetBufferedRegion().GetNumberOfPixels()) * 0.01) );
    }


    metric->Initialize();
    std::cout << "size: " << metric->GetFixedImageRegion() << std::endl;
  }
  registration->SetFixedImageRegion ( fixedImageReader->GetOutput()->GetBufferedRegion() );

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
  if (!fixedImageIsBigger)
  {
    metric->SetNumberOfSpatialSamples ( static_cast<int> ( nPixels * 0.01 ) );
  }

  metric->SetNumberOfHistogramBins ( nbins );

  if (command.GetOptionWasSet("threshold_rel") or command.GetOptionWasSet("loadmask") or command.GetOptionWasSet("threshold_abs") or command.GetOptionWasSet("otsuMask"))
  {
    metric->SetFixedImageMask( maskObject );
  }
  std::cout << "spatial samples: " << metric->GetNumberOfSpatialSamples() << std::endl;

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

  const double versorX = finalParameters[0];
  const double versorY = finalParameters[1];
  const double versorZ = finalParameters[2];
  const double finalTranslationX = finalParameters[3];
  const double finalTranslationY = finalParameters[4];
  const double finalTranslationZ = finalParameters[5];

  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();

  const double bestValue = optimizer->GetValue();

  // Print out results

  std::cout << std::endl << std::endl;
  std::cout << "Metric: Number of bins = " << nbins << std::endl;
  std::cout << "Result = " << std::endl;
  std::cout << " versor X      = " << versorX << std::endl;
  std::cout << " versor Y      = " << versorY << std::endl;
  std::cout << " versor Z      = " << versorZ << std::endl;
  std::cout << " Translation X = " << finalTranslationX << std::endl;
  std::cout << " Translation Y = " << finalTranslationY << std::endl;
  std::cout << " Translation Z = " << finalTranslationZ << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue << std::endl;

  /**************************************************
   * resample image
   *************************************************/
  FixedImageType::Pointer fixedImage = fixedImageReader->GetOutput();

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
