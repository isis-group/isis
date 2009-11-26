/*
 * isisKaltmacher.cxx
 *
 *  Created on: November 26, 2009
 *      Author: tuerke
 */

//MACHE EIER JUNGE


#include "viaio/option.h"
#include "viaio/mu.h"

//itk related includes
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkImageRegistrationMethod.h"
#include "itkVersorRigid3DTransform.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"

#include "extRegistration/isisIterationObserver.h"

//command line parser options
static VString ref_filename = NULL;
static VString in_filename = NULL;
static VBoolean resample = false;
static VString out_filename = NULL;
static VBoolean in_found, ref_found;

//optimizer related parser options
static VArgVector scales;
static VFloat minStep = 0.0001;
static VFloat maxStep = 0.1;
static VFloat gmt = 1e-6;
static VFloat relax = 0.5;
static VShort iter = 5000;

static VOptionDescRec
options[] = {
    //required inputs
    {"ref", VStringRepn, 1, &ref_filename, &ref_found, 0, "the fixed image filename"},
    {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the moving image filename"},
    {"scales", VFloatRepn, 0, (VPointer) &scales, VOptionalOpt, 0, "vector of the scales"},
    {"minStep", VFloatRepn, 1, &minStep, VOptionalOpt, 0, ""},
    {"maxStep", VFloatRepn, 1, &maxStep, VOptionalOpt, 0, ""},
    {"gmt", VFloatRepn, 1, &gmt, VOptionalOpt, 0, ""},
    {"iter", VShortRepn, 1, &iter, VOptionalOpt, 0, ""},
    {"relax", VFloatRepn, 1, &relax, VOptionalOpt, 0, ""}
};

int main(int argc, char* argv[])
{
  // show revision information string constant
    std::cout << "Revision: " << SVN_REVISION << std::endl;


    // DANGER! Kids don't try this at home! VParseCommand modifies the values of argc and argv!!!
    if (!VParseCommand(VNumber(options), options, &argc, argv) || !VIdentifyFiles(VNumber(options), options, "in",
            &argc, argv, 0)) {
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(1);
    }

    // VParseCommand reduces the argv vector to the name of the program and  unknown command line parameters.
    if (argc > 1) {
        VReportBadArgs(argc, argv);
        VReportUsage(argv[0], VNumber(options), options, NULL);
        exit(1);
    }
  
  if(scales.number < 6)
  {
    std::cout << "the number of scales has to be 6!" << std::endl;
    return EXIT_FAILURE;
  }
  
  const unsigned int Dimension = 3;
  typedef signed short PixelType;
  
  typedef itk::Image<PixelType, Dimension> ImageType;
  typedef itk::ImageFileReader<ImageType> ImageFileReaderType;
  typedef itk::ImageFileWriter<ImageType> ImageFileWriterType;
  
  typedef itk::ImageRegistrationMethod<ImageType, ImageType> ImageRegistrationMethodType;
  typedef itk::LinearInterpolateImageFunction<ImageType, double> InterpolateImageFunctionType;
  typedef itk::MattesMutualInformationImageToImageMetric<ImageType, ImageType> MattesMutualInformationImageToImageMetricType;
  typedef itk::VersorRigid3DTransformOptimizer VersorRigid3DTransformOptimizerType;
  typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
  
  ImageFileReaderType::Pointer fixedReader = ImageFileReaderType::New();
  ImageFileReaderType::Pointer movingReader = ImageFileReaderType::New();
  ImageFileWriterType::Pointer writer = ImageFileWriterType::New();
  
  ImageRegistrationMethodType::Pointer registrationMethod = ImageRegistrationMethodType::New();
  InterpolateImageFunctionType::Pointer interpolator = InterpolateImageFunctionType::New();
  MattesMutualInformationImageToImageMetricType::Pointer metric = MattesMutualInformationImageToImageMetricType::New();
  VersorRigid3DTransformOptimizerType::Pointer optimizer = VersorRigid3DTransformOptimizerType::New();
  VersorRigid3DTransformType::Pointer transform = VersorRigid3DTransformType::New();
 
  isis::IterationObserver::Pointer observer = isis::IterationObserver::New();
  
  fixedReader->SetFileName(ref_filename);
  movingReader->SetFileName(in_filename);
  fixedReader->Update();
  movingReader->Update();
  
  //set up the registrion method itself
  registrationMethod->SetFixedImage(fixedReader->GetOutput());
  registrationMethod->SetMovingImage(movingReader->GetOutput());
  registrationMethod->SetFixedImageRegion(fixedReader->GetOutput()->GetLargestPossibleRegion());
  registrationMethod->SetInterpolator(interpolator);
  registrationMethod->SetMetric(metric);
  registrationMethod->SetOptimizer(optimizer);
  registrationMethod->SetTransform(transform);
  registrationMethod->SetInitialTransformParameters(transform->GetParameters());
  
  //set up the optimizer
  
  VersorRigid3DTransformOptimizerType::ScalesType optimizerScale(transform->GetNumberOfParameters());
  for(unsigned int i = 0; i<6; i++)
  {
    
    optimizerScale[i] = ((VFloat*) scales.vector)[i];
    
  }
 
  optimizer->SetScales(optimizerScale);
  optimizer->SetGradientMagnitudeTolerance(gmt);
  optimizer->SetMaximumStepLength(maxStep);
  optimizer->SetMinimumStepLength(minStep);
  optimizer->SetNumberOfIterations(iter);
  optimizer->SetRelaxationFactor(relax);
  
  //set up the metric
  metric->SetFixedImage(fixedReader->GetOutput());
  metric->SetMovingImage(movingReader->GetOutput());
  metric->SetFixedImageRegion(fixedReader->GetOutput()->GetLargestPossibleRegion());
  metric->SetNumberOfHistogramBins(50);
  metric->SetNumberOfSpatialSamples(fixedReader->GetOutput()->GetLargestPossibleRegion().GetNumberOfPixels()/100);
  metric->ReinitializeSeed(1);
  
  
  optimizer->AddObserver(itk::IterationEvent(), observer);
  
  registrationMethod->StartRegistration();
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  return 0;
};