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
#include "itkTransformFileWriter.h"

#include "itkImageRegistrationMethod.h"
#include "itkVersorRigid3DTransform.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedMutualInformationHistogramImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"

#include "extITK/isisIterationObserver.h"
#include <iostream>
#include "itkMeanImageFilter.h"
#include "itkMedianImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"

//command line parser options
static VString ref_filename = NULL;
static VString in_filename = NULL;
//static VBoolean resample = false;
static VString out_filename = NULL;
static VBoolean in_found, ref_found, out_found;

//optimizer related parser options
static VArgVector scales;
static VFloat minStep = 0.0001;
static VFloat maxStep = 0.1;
static VFloat gmt = 1e-6;
static VFloat relax = 0.5;
static VShort iter = 5000;
static VFloat radius = 1;

static VOptionDescRec
options[] = {
    //required inputs
    {"ref", VStringRepn, 1, &ref_filename, &ref_found, 0, "the fixed image filename"},
    {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the moving image filename"},
    {"out", VStringRepn, 1, &out_filename, &out_found, 0, "the output transformation filename"},
    {"scales", VFloatRepn, 0, (VPointer) &scales, VOptionalOpt, 0, "vector of the scales"},
    {"minStep", VFloatRepn, 1, &minStep, VOptionalOpt, 0, ""},
    {"maxStep", VFloatRepn, 1, &maxStep, VOptionalOpt, 0, ""},
    {"gmt", VFloatRepn, 1, &gmt, VOptionalOpt, 0, ""},
    {"iter", VShortRepn, 1, &iter, VOptionalOpt, 0, ""},
    {"relax", VFloatRepn, 1, &relax, VOptionalOpt, 0, ""},
	{"radius", VFloatRepn, 1, &radius, VOptionalOpt, 0, ""}
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
	typedef itk::MattesMutualInformationImageToImageMetric<ImageType, ImageType>
	                MattesMutualInformationImageToImageMetricType;
	typedef itk::NormalizedMutualInformationHistogramImageToImageMetric<ImageType, ImageType> NormalizedMutualInformationMetricType;
	typedef itk::VersorRigid3DTransformOptimizer VersorRigid3DTransformOptimizerType;
	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::MeanImageFilter<ImageType, ImageType> MeanImageFilterType;
	typedef itk::MedianImageFilter<ImageType, ImageType> MedianImageFilterType;
	typedef itk::DiscreteGaussianImageFilter<ImageType, ImageType> DiscreteGaussianImageFilterType;

	//  Transformation
	itk::TransformFileWriter::Pointer transformWriter = itk::TransformFileWriter::New();
	const itk::TransformBase* tmpTransform;

	ImageFileReaderType::Pointer fixedReader = ImageFileReaderType::New();
	ImageFileReaderType::Pointer movingReader = ImageFileReaderType::New();
	ImageFileWriterType::Pointer writer = ImageFileWriterType::New();

	ImageRegistrationMethodType::Pointer registrationMethod = ImageRegistrationMethodType::New();
	InterpolateImageFunctionType::Pointer interpolator = InterpolateImageFunctionType::New();
	MattesMutualInformationImageToImageMetricType::Pointer metric =
	                MattesMutualInformationImageToImageMetricType::New();
	NormalizedMutualInformationMetricType::Pointer normalizedMetric = NormalizedMutualInformationMetricType::New();
	VersorRigid3DTransformOptimizerType::Pointer optimizer =
	                VersorRigid3DTransformOptimizerType::New();
	VersorRigid3DTransformType::Pointer transform = VersorRigid3DTransformType::New();

	ImageType::Pointer fixedImage = ImageType::New();
	ImageType::Pointer movingImage = ImageType::New();
	
	MeanImageFilterType::Pointer filter1 = MeanImageFilterType::New();
	MeanImageFilterType::Pointer filter2 = MeanImageFilterType::New();
	MedianImageFilterType::Pointer medianFilter1 = MedianImageFilterType::New();
	MedianImageFilterType::Pointer medianFilter2 = MedianImageFilterType::New();
	DiscreteGaussianImageFilterType::Pointer discreteFilter1 = DiscreteGaussianImageFilterType::New();
	DiscreteGaussianImageFilterType::Pointer discreteFilter2 = DiscreteGaussianImageFilterType::New();
	
	isis::extitk::IterationObserver::Pointer observer = isis::extitk::IterationObserver::New();

	fixedReader->SetFileName(ref_filename);
	movingReader->SetFileName(in_filename);
	fixedReader->Update();
	movingReader->Update();
/*
	ImageType::SizeType indexRadius;
	indexRadius.Fill(radius);
	medianFilter1->SetRadius(indexRadius);
	medianFilter2->SetRadius(indexRadius);
	medianFilter1->SetInput(fixedReader->GetOutput());
	medianFilter1->Update();
	fixedImage = medianFilter1->GetOutput();
	medianFilter2->SetInput(movingReader->GetOutput());
	medianFilter2->Update();
	movingImage = medianFilter2->GetOutput();
	*/

	discreteFilter1->SetVariance(1);
	discreteFilter2->SetVariance(1);
	discreteFilter1->SetMaximumKernelWidth(radius);
	discreteFilter2->SetMaximumKernelWidth(radius);
	discreteFilter1->SetInput(fixedReader->GetOutput());
	discreteFilter2->SetInput(movingReader->GetOutput());
	discreteFilter1->Update();
	discreteFilter2->Update();
	fixedImage = discreteFilter1->GetOutput();
	movingImage = discreteFilter2->GetOutput();
	
	//set up the registrion method itself
	registrationMethod->SetFixedImage(fixedImage);
	registrationMethod->SetMovingImage(movingImage);
	registrationMethod->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());
	registrationMethod->SetInterpolator(interpolator);
	registrationMethod->SetMetric(metric);
	registrationMethod->SetOptimizer(optimizer);
	registrationMethod->SetTransform(transform);
	registrationMethod->SetInitialTransformParameters(transform->GetParameters());

	//set up the optimizer

  VersorRigid3DTransformOptimizerType::ScalesType optimizerScale(
	    transform->GetNumberOfParameters());
	for(unsigned int i = 0; i < 6; i++)
	{

		optimizerScale[i] = ((VFloat*) scales.vector)[i];

	}

	optimizer->SetScales(optimizerScale);
	optimizer->SetGradientMagnitudeTolerance(gmt);
	optimizer->SetMaximumStepLength(maxStep);
	optimizer->SetMinimumStepLength(minStep);
	optimizer->SetNumberOfIterations(iter);
	optimizer->SetRelaxationFactor(relax);
	optimizer->MinimizeOn();
	
	//set up the metric
	metric->SetFixedImage(fixedImage);
	metric->SetMovingImage(movingImage);
	metric->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());
	metric->SetNumberOfHistogramBins(256);
	metric->SetNumberOfSpatialSamples(fixedImage->GetLargestPossibleRegion().GetNumberOfPixels() / 100);
	metric->ReinitializeSeed(76321);
	
	//set up the normalized metric
	NormalizedMutualInformationMetricType::ScalesType scales(transform->GetNumberOfParameters());
	NormalizedMutualInformationMetricType::HistogramType::SizeType histogramSize;
	histogramSize[0] = 25;
	histogramSize[1] = 25;
	normalizedMetric->SetHistogramSize(histogramSize);
	scales.Fill(0.001);
	normalizedMetric->SetDerivativeStepLengthScales(scales);
	normalizedMetric->SetFixedImage(fixedImage);
	normalizedMetric->SetMovingImage(movingImage);
	normalizedMetric->SetFixedImageRegion(fixedImage->GetLargestPossibleRegion());
	normalizedMetric->SetPaddingValue(10);
	
	

	optimizer->AddObserver(itk::IterationEvent(), observer);
	registrationMethod->StartRegistration();

	//  if output filename found -> save transformation to disk
	if(out_filename && out_found)
	{
		std::cout << "Writing transformation to disk" << std::endl;

		tmpTransform = registrationMethod->GetTransform();
		transformWriter->SetInput(tmpTransform);
		transformWriter->SetFileName(out_filename);
		transformWriter->Update();
	}

	return 0;
};
