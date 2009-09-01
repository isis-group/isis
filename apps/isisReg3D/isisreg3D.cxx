/*
 * isisRegistration.cxx
 *
 *  Created on: July 13, 2009
 *      Author: tuerke
 */

#include "extRegistration/isisRegistrationFactory3D.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

#include "itkCheckerBoardImageFilter.h"

//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber
VDictEntry TYPMetric[] = { { "MattesMutualInformation", 0 }, {
		"ViolaWellsMutualInformation", 1 }, { "NormalizedCorrelation", 2 }, {
		NULL } };

VDictEntry TYPTransform[] = { { "Rigid", 0 }, { "Affine", 1 }, {
		"CenteredAffine", 2 }, { "BSplineDeformable", 3 }, { "QuaternionRigid",
		4 }, { "EulerRigid", 5 }, { NULL } };

VDictEntry TYPInterpolator[] = { { "Linear", 0 }, { "BSpline", 1 }, { NULL } };

VDictEntry TYPOptimizer[] = { { "RegularStepGradientDescent", 0 }, {
		"VersorRigid", 1 }, { "LBFGSB", 2 }, { NULL } };

//command line parser options
static VString ref_filename = NULL;
static VString in_filename = NULL;
static VString out_filename = NULL;
static VShort number_of_bins = 50;
static VShort number_of_iterations = 200;
static VFloat pixel_density = 0.01;
static VShort metricType = 0;
static VShort transformType = 0;
static VShort interpolatorType = 0;
static VShort optimizerType = 0;
static VBoolean in_found, out_found, ref_found;
static VShort checker_parts = 0;

static VOptionDescRec
		options[] = {
				//required inputs
				{ "ref", VStringRepn, 1, &ref_filename, &ref_found, 0,
						"the fixed image filename" },
				{ "in", VStringRepn, 1, &in_filename, &in_found, 0,
						"the moving image filename" },
				{ "out", VStringRepn, 1, &out_filename, &out_found, 0,
						"the output image filename" },

				//parameter inputs
				{
						"bins",
						VShortRepn,
						1,
						&number_of_bins,
						VOptionalOpt,
						0,
						"Number of bins used by the MattesMutualInformationMetric to calculate the image histogram" },
				{ "iter", VShortRepn, 1, &number_of_iterations, VOptionalOpt,
						0, "Maximum number of iteration used by the optimizer" },
				{ "cb", VShortRepn, 1, &checker_parts, VOptionalOpt, 0,
						"Number of patterns in each dimension" },

				{
						"pd",
						VFloatRepn,
						1,
						&pixel_density,
						VOptionalOpt,
						0,
						"The density of pixels the metric uses. 1 denotes the metric uses all pixels. Has to be > 0. Only operative with a MattesMutualInformation metric" },

				//component inputs
				{ "metric", VShortRepn, 1, (VPointer) &metricType,
						VOptionalOpt, TYPMetric, "Type of the metric" },
				{ "transform", VShortRepn, 1, (VPointer) &transformType,
						VOptionalOpt, TYPTransform, "Type of the transform" },
				{ "interpolator", VShortRepn, 1, (VPointer) &interpolatorType,
						VOptionalOpt, TYPInterpolator, "Type of interpolator" },
				{ "optimizer", VShortRepn, 1, (VPointer) &optimizerType,
						VOptionalOpt, TYPOptimizer, "Type of optimizer" }

		};

// This is the main function
int main(int argc, char* argv[]) {

	// DANGER! Kids don't try this at home! VParseCommand modifies the values of argc and argv!!!
	if (!VParseCommand(VNumber(options), options, &argc, argv)
			|| !VIdentifyFiles(VNumber(options), options, "in", &argc, argv, 0)
			|| !VIdentifyFiles(VNumber(options), options, "out", &argc, argv,
					-1)) {
		VReportUsage(argv[0], VNumber(options), options, NULL);
		exit(1);
	}

	// VParseCommand reduces the argv vector to the name of the program and  unknown command line parameters.
	if (argc > 1) {
		VReportBadArgs(argc, argv);
		VReportUsage(argv[0], VNumber(options), options, NULL);
		exit(1);
	}

	//check pixel density
	if (pixel_density <= 0) {
		std::cerr << "wrong pixel density...set to 0.01" << std::endl;
		pixel_density = 0.01;
	}
	if (pixel_density >= 1) {
		std::cerr << "metric uses all pixels" << std::endl;
	}

	//check combinations of components
	if ((optimizerType == 1 and transformType != 0)) {
		std::cerr
				<< "\nInappropriate combination of transform and optimizer! Setting optimizer to RegularStepGradientDescent.\n"
				<< std::endl;
		optimizerType = 0;
	}

	typedef short InputPixelType;
	typedef short OutputPixelType;
	const unsigned int Dimension = 3;

	typedef itk::Image<InputPixelType, Dimension> FixedImageType;
	typedef itk::Image<InputPixelType, Dimension> MovingImageType;
	typedef itk::Image<OutputPixelType, Dimension> OutputImageType;

	typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
	typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType;
	typedef itk::ImageFileWriter<OutputImageType> WriterType;

	typedef isis::RegistrationFactory3D<FixedImageType, MovingImageType>
			RegistrationFactoryType;

	typedef itk::CheckerBoardImageFilter<FixedImageType> CheckerBoardFilterType;

	CheckerBoardFilterType::Pointer checker = CheckerBoardFilterType::New();
	FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
	MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	fixedReader->SetFileName(ref_filename);
	movingReader->SetFileName(in_filename);
	writer->SetFileName(out_filename);

	fixedReader->Update();
	movingReader->Update();

	RegistrationFactoryType::Pointer registrationFactory =
			RegistrationFactoryType::New();

	//transform setup
	std::cout << "used transform: " << TYPTransform[transformType].keyword
			<< std::endl;
	switch (transformType) {
	case 0:
		registrationFactory->SetTransform(
				RegistrationFactoryType::VersorRigid3DTransform);
		break;
	case 1:
		registrationFactory->SetTransform(
				RegistrationFactoryType::AffineTransform);
		break;
	case 2:
		registrationFactory->SetTransform(
				RegistrationFactoryType::CenteredAffineTransform);
		break;
	case 3:
		registrationFactory->SetTransform(
				RegistrationFactoryType::BSplineDeformableTransform);
		break;
	case 4:
		registrationFactory->SetTransform(
				RegistrationFactoryType::QuaternionRigidTransform);
		break;
	case 5:
		registrationFactory->SetTransform(
				RegistrationFactoryType::CenteredEuler3DTransform);
		break;
	}

	//metric setup
	std::cout << "used metric: " << TYPMetric[metricType].keyword << std::endl;
	switch (metricType) {
	case 0:
		registrationFactory->SetMetric(
				RegistrationFactoryType::MattesMutualInformation);
		break;

	case 1:
		registrationFactory->SetMetric(
				RegistrationFactoryType::ViolaWellsMutualInformation);
		break;

	case 2:
		registrationFactory->SetMetric(
				RegistrationFactoryType::NormalizedCorrelation);
		break;
	}

	//interpolator setup
	std::cout << "used interpolator: "
			<< TYPInterpolator[interpolatorType].keyword << std::endl;
	switch (interpolatorType) {
	case 0:
		registrationFactory->SetInterpolator(
				RegistrationFactoryType::LinearInterpolator);
		break;
	case 1:
		registrationFactory->SetInterpolator(
				RegistrationFactoryType::BSplineInterpolator);
		break;
	}
	//optimizer setup
	std::cout << "used optimizer: " << TYPOptimizer[optimizerType].keyword
			<< std::endl;

	switch (optimizerType) {
	case 0:
		registrationFactory->SetOptimizer(
				RegistrationFactoryType::RegularStepGradientDescentOptimizer);
		break;
	case 1:
		registrationFactory->SetOptimizer(
				RegistrationFactoryType::VersorRigidOptimizer);
		break;
	case 2:
		registrationFactory->SetOptimizer(
				RegistrationFactoryType::LBFGSBOptimizer);
		break;
	}

	registrationFactory->UserOptions.NumberOfIterations = number_of_iterations;
	registrationFactory->UserOptions.NumberOfBins = number_of_bins;
	registrationFactory->UserOptions.PixelDensity = pixel_density;
	registrationFactory->UserOptions.PRINTRESULTS = true;

	registrationFactory->SetFixedImage(fixedReader->GetOutput());
	registrationFactory->SetMovingImage(movingReader->GetOutput());
	registrationFactory->StartRegistration();

	writer->SetInput(registrationFactory->GetRegisteredImage());
	writer->Update();

	//checkerboard filter
	if (checker_parts > 0) {
		CheckerBoardFilterType::PatternArrayType patterns;
		for (int i = 0; i < 3; i++) {
			patterns[i] = checker_parts;
		}
		checker->SetInput1(fixedReader->GetOutput());
		checker->SetInput2(registrationFactory->GetRegisteredImage());
		checker->SetCheckerPattern(patterns);
		checker->Update();
		writer->SetFileName("checkerboard.nii");
		writer->SetInput(checker->GetOutput());
		writer->Update();

	}

	return 0;

}
