/*
 * isisreg3D.cxx
 *
 *  Created on: July 13, 2009
 *      Author: tuerke
 */

#include "itkWarpImageFilter.h"
#include "itkImageMaskSpatialObject.h"

#include "extRegistration/isisRegistrationFactory2D.h"
#include "extITK/isisTransformMerger2D.hpp"
#include "extITK/isisIterationObserver.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"

#include "itkTransformFileWriter.h"
#include "itkTransformFileReader.h"

#include <fstream>
#include "boost/algorithm/string.hpp"

#include "itkLandmarkBasedTransformInitializer.h"
#include "itkRigid2DTransform.h"

#include "itkMedianImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"

#include "boost/progress.hpp"

//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber
VDictEntry TYPMetric[] = { {"MattesMutualInformation", 0}, {"MutualInformationHistogram", 1}, {"NormalizedCorrelation",
    2}, {"MeanSquare", 3}, {NULL}};

VDictEntry TYPTransform[] = { {"Rigid", 0}, {"Affine", 1}, {"BSplineDeformable", 2}, {"Translation", 3}, {"CenteredAffine", 4}, {NULL}};

VDictEntry TYPInterpolator[] = { {"Linear", 0}, {"BSpline", 1}, {"NearestNeighbor", 2}, {NULL}};

VDictEntry TYPOptimizer[] = { {"RegularStepGradientDescent", 0}, {"LBFGSB", 1}, {"Amoeba", 2}, {
    "Powell", 3}, {NULL}};

//command line parser options
static VString ref_filename = NULL;
static VString in_filename = NULL;
static VString out_filename = NULL;
static VString vout_filename = NULL;
static VString pointset_filename = NULL;
static VString transform_filename_in = NULL;
static VShort number_of_bins = 50;
static VShort number_of_iterations = 1000;
static VFloat pixel_density = 0.01;
static VShort grid_size = 5;
static VShort metricType = 0;
static VArgVector transformType;
static VArgVector interpolatorType;
static VArgVector optimizerType;
static VBoolean in_found, ref_found, pointset_found;
static VShort number_threads = 1;
static VShort initial_seed = 1;
static VBoolean initialize_center = false;
static VBoolean initialize_mass = false;
static VString mask_filename = NULL;
static VFloat smooth = 0;
static VBoolean use_inverse = false;

static VOptionDescRec
        options[] = {
            //required inputs
            {"ref", VStringRepn, 1, &ref_filename, &ref_found, 0, "the fixed image filename"},
            {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the moving image filename"},

            //non-required inputs
            {"mask", VStringRepn, 1, &mask_filename, VOptionalOpt, 0, "the mask filename"},
            {"pointset", VStringRepn, 1, &pointset_filename, &pointset_found, 0, "the pointset filename"},
            {"out", VStringRepn, 1, &out_filename, VOptionalOpt, 0, "the output transform filename"},
            {"vout", VStringRepn, 1, &vout_filename, VOptionalOpt, 0, "the output vector image filename"},
            {"tin", VStringRepn, 1, &transform_filename_in, VOptionalOpt, 0,
                "filename of the transform used as an initial transform"},
            //parameter inputs
            {"bins", VShortRepn, 1, &number_of_bins, VOptionalOpt, 0,
                "Number of bins used by the MattesMutualInformationMetric to calculate the image histogram"},
            {"iter", VShortRepn, 1, &number_of_iterations, VOptionalOpt, 0,
                "Maximum number of iteration used by the optimizer"},
            {"seed", VShortRepn, 1, &initial_seed, VOptionalOpt, 0,
                "The initialize seed for the MattesMutualInformationMetric"},

            {
                "pd",
                VFloatRepn,
                1,
                &pixel_density,
                VOptionalOpt,
                0,
                "The density of pixels the metric uses. 1 denotes the metric uses all pixels. Has to be > 0. Only operative with a MattesMutualInformation metric"},

            {"j", VShortRepn, 1, &number_threads, VOptionalOpt, 0, "Number of threads used for the registration"},

            {"gridSize", VShortRepn, 1, &grid_size, VOptionalOpt, 0,
                "Grid size used for the BSplineDeformable transform."},

            {"prealign_center", VBooleanRepn, 1, &initialize_center, VOptionalOpt, 0,
                "Using an initializer to align the image centers"},
            {"prealign_mass", VBooleanRepn, 1, &initialize_mass, VOptionalOpt, 0,
                                "Using an initializer to align the center of mass"},
	    {"smooth", VFloatRepn, 1, &smooth, VOptionalOpt, 0, "Applying a smoothing filter to the fixed and moving image before the registration process"},
	    {"get_inverse", VBooleanRepn, 1, &use_inverse, VOptionalOpt, 0, "Getting the inverse transform"},

            //component inputs
            {"metric", VShortRepn, 1, (VPointer) &metricType, VOptionalOpt, TYPMetric, "Type of the metric"}, {
                "transform", VShortRepn, 0, (VPointer) &transformType, VOptionalOpt, TYPTransform,
                "Type of the transform"}, {"interpolator", VShortRepn, 0, (VPointer) &interpolatorType, VOptionalOpt,
                TYPInterpolator, "Type of interpolator"}, {"optimizer", VShortRepn, 0, (VPointer) &optimizerType,
                VOptionalOpt, TYPOptimizer, "Type of optimizer"}

        };

// This is the main function
int main(
    int argc, char* argv[]) {

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

	typedef unsigned char MaskPixelType;
	typedef unsigned char InputPixelType;
	const unsigned int Dimension = 2;

	VShort transform;
	VShort optimizer;
	VShort interpolator;

	typedef itk::Image<InputPixelType, Dimension> FixedImageType;
	typedef itk::Image<InputPixelType, Dimension> MovingImageType;
	typedef itk::Image<MaskPixelType, Dimension> MaskImageType;
	typedef itk::ImageMaskSpatialObject<Dimension> ImageMaskSpatialObjectType;

	typedef itk::ImageFileReader<FixedImageType> FixedImageReaderType;
	typedef itk::ImageFileReader<MovingImageType> MovingImageReaderType;
	typedef itk::ImageFileReader<MaskImageType> MaskImageReaderType;

	typedef isis::registration::RegistrationFactory2D<FixedImageType, MovingImageType> RegistrationFactoryType;

	typedef itk::Vector<float, Dimension> VectorType;
	typedef itk::Image<VectorType, Dimension> DeformationFieldType;

	typedef itk::Rigid2DTransform<double> Rigid2DTransformType;

	typedef itk::PointSet<float, Dimension> PointSetType;

	typedef itk::LandmarkBasedTransformInitializer<Rigid2DTransformType, FixedImageType, MovingImageType>
	LandmarkBasedTransformInitializerType;

	typedef itk::ImageFileWriter<DeformationFieldType> VectorWriterType;

	const itk::TransformBase* tmpConstTransformPointer;
	typedef itk::TransformBase* TransformBasePointerType;

	typedef itk::MedianImageFilter<FixedImageType, FixedImageType> FixedFilterType;
	typedef itk::MedianImageFilter<MovingImageType, MovingImageType> MovingFilterType;
	
	typedef itk::RecursiveGaussianImageFilter<FixedImageType, FixedImageType> GaussianFilterType;

	FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
	MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
	MaskImageReaderType::Pointer maskReader = MaskImageReaderType::New();
	
	itk::AffineTransform<double, Dimension>::Pointer tmpTransform = itk::AffineTransform<double, Dimension>::New();

	itk::TransformFileWriter::Pointer transformWriter = itk::TransformFileWriter::New();
	VectorWriterType::Pointer vectorWriter = VectorWriterType::New();
	itk::TransformFileReader::Pointer transformReader = itk::TransformFileReader::New();

	ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
	FixedImageType::Pointer fixedImage = FixedImageType::New();
	MovingImageType::Pointer movingImage = MovingImageType::New();
	
	FixedFilterType::Pointer fixedFilter = FixedFilterType::New();
	MovingFilterType::Pointer movingFilter = MovingFilterType::New();

	fixedReader->SetFileName(ref_filename);
	movingReader->SetFileName(in_filename);

	fixedReader->Update();
	movingReader->Update();
	
	if(!smooth) {
	    fixedImage = fixedReader->GetOutput();
	    movingImage = movingReader->GetOutput();
	}
	
	if(smooth) {
	    GaussianFilterType::Pointer fixedGaussianFilterX = GaussianFilterType::New();
	    GaussianFilterType::Pointer fixedGaussianFilterY = GaussianFilterType::New();
	    fixedGaussianFilterX->SetNumberOfThreads(number_threads);
	    fixedGaussianFilterY->SetNumberOfThreads(number_threads);
	    fixedGaussianFilterX->SetInput(fixedReader->GetOutput());
	    fixedGaussianFilterY->SetInput(fixedGaussianFilterX->GetOutput());
	    fixedGaussianFilterX->SetDirection(0);
	    fixedGaussianFilterY->SetDirection(1);
	    fixedGaussianFilterX->SetOrder(GaussianFilterType::ZeroOrder);
	    fixedGaussianFilterY->SetOrder(GaussianFilterType::ZeroOrder);
	    fixedGaussianFilterX->SetNormalizeAcrossScale(false);
	    fixedGaussianFilterY->SetNormalizeAcrossScale(false);
	    fixedGaussianFilterX->SetSigma(smooth);
	    fixedGaussianFilterY->SetSigma(smooth);
	    std::cout << "smoothing the fixed image..." << std::endl;
	    fixedGaussianFilterY->Update();
	    fixedImage = fixedGaussianFilterY->GetOutput();
	    GaussianFilterType::Pointer movingGaussianFilterX = GaussianFilterType::New();
	    GaussianFilterType::Pointer movingGaussianFilterY = GaussianFilterType::New();
	    movingGaussianFilterX->SetNumberOfThreads(number_threads);
	    movingGaussianFilterY->SetNumberOfThreads(number_threads);
	    movingGaussianFilterX->SetInput(movingReader->GetOutput());
	    movingGaussianFilterY->SetInput(movingGaussianFilterX->GetOutput());
	    movingGaussianFilterX->SetDirection(0);
	    movingGaussianFilterY->SetDirection(1);
	    movingGaussianFilterX->SetOrder(GaussianFilterType::ZeroOrder);
	    movingGaussianFilterY->SetOrder(GaussianFilterType::ZeroOrder);
	    movingGaussianFilterX->SetNormalizeAcrossScale(false);
	    movingGaussianFilterY->SetNormalizeAcrossScale(false);
	    movingGaussianFilterX->SetSigma(smooth);
	    movingGaussianFilterY->SetSigma(smooth);
	    std::cout << "smoothing the moving image..." << std::endl;
	    movingGaussianFilterY->Update();
	    movingImage = movingGaussianFilterY->GetOutput();	     
	}
	
	RegistrationFactoryType::Pointer registrationFactory = RegistrationFactoryType::New();
	isis::extitk::TransformMerger2D* transformMerger = new isis::extitk::TransformMerger2D;

	//analyse transform vector
	unsigned int repetition = transformType.number;
	if (!repetition)
	repetition = 1;
	//analyse optimizer vector

	boost::progress_timer time_used;
	for (unsigned int counter = 0; counter < repetition; counter++) {

		if (transformType.number) {
			transform = ((VShort*) transformType.vector)[counter];
		} else {
			transform = 0;
		}
		if (optimizerType.number) {
			optimizer = ((VShort*) optimizerType.vector)[counter];
		} else {
			optimizer = 0;
		}
		if (interpolatorType.number) {
			 interpolator = ((VShort*) interpolatorType.vector)[counter];
		} else {
		    interpolator = 0;
		}

		std::cout << std::endl << "setting up the registration object..." << std::endl;
		registrationFactory->Reset();

		//check pixel density
		if (pixel_density <= 0) {
			std::cerr << "wrong pixel density...set to 0.01" << std::endl;
			pixel_density = 0.01;
		}
		if (pixel_density >= 1) {
			std::cerr << "metric uses all pixels" << std::endl;
		}

		//check grid size
		if (grid_size <= 4) {
			std::cerr << "\ngrid size has to be bigger than 4...setting grid size to 5\n" << std::endl;
			grid_size = 5;
		}

		if (transform == 2 and optimizer != 1) {
			std::cerr << "\nIt is recommended using the BSpline transform in connection with the LBFGSB optimizer!\n"
			<< std::endl;
		}

		//transform setup
		std::cout << "used transform: " << TYPTransform[transform].keyword << std::endl;
		switch (transform) {
			case 0:
			registrationFactory->SetTransform(RegistrationFactoryType::Rigid2DTransform);
			break;
			case 1:
			registrationFactory->SetTransform(RegistrationFactoryType::AffineTransform);
			break;
			case 2:
			registrationFactory->SetTransform(RegistrationFactoryType::BSplineDeformableTransform);
			break;
			case 3:
			registrationFactory->SetTransform(RegistrationFactoryType::TranslationTransform);
			break;
		}

		//metric setup
		std::cout << "used metric: " << TYPMetric[metricType].keyword << std::endl;
		switch (metricType) {
			case 0:
			registrationFactory->SetMetric(RegistrationFactoryType::MattesMutualInformationMetric);
			break;
			case 1:
			registrationFactory->SetMetric(RegistrationFactoryType::MutualInformationHistogramMetric);
			break;
			case 2:
			registrationFactory->SetMetric(RegistrationFactoryType::NormalizedCorrelationMetric);
			break;
			case 3:
			registrationFactory->SetMetric(RegistrationFactoryType::MeanSquareMetric);
			break;

		}

		//interpolator setup
		std::cout << "used interpolator: " << TYPInterpolator[interpolator].keyword << std::endl;
		switch (interpolator) {
			case 0:
			registrationFactory->SetInterpolator(RegistrationFactoryType::LinearInterpolator);
			break;
			case 1:
			registrationFactory->SetInterpolator(RegistrationFactoryType::BSplineInterpolator);
			break;
			case 2:
			registrationFactory->SetInterpolator(RegistrationFactoryType::NearestNeighborInterpolator);
			break;
		}
		//optimizer setup
		std::cout << "used optimizer: " << TYPOptimizer[optimizer].keyword << std::endl;

		switch (optimizer) {
			case 0:
			registrationFactory->SetOptimizer(RegistrationFactoryType::RegularStepGradientDescentOptimizer);
			break;
			case 1:
			registrationFactory->SetOptimizer(RegistrationFactoryType::LBFGSBOptimizer);
			break;
			case 2:
			registrationFactory->SetOptimizer(RegistrationFactoryType::AmoebaOptimizer);
			break;
			case 3:
			registrationFactory->SetOptimizer(RegistrationFactoryType::PowellOptimizer);
			break;
		}

		if (transform_filename_in and counter == 0) {
			transformReader->SetFileName(transform_filename_in);
			transformReader->Update();

			itk::TransformFileReader::TransformListType *transformList = transformReader->GetTransformList();
			itk::TransformFileReader::TransformListType::const_iterator ti;
			ti = transformList->begin();
			registrationFactory->SetInitialTransform((*ti).GetPointer());

		}
		if (counter != 0) {
			registrationFactory->UserOptions.INITIALIZECENTEROFF = true;
			registrationFactory->UserOptions.INITIALIZEMASSOFF = true;
			registrationFactory->SetInitialTransform(const_cast<TransformBasePointerType> (tmpConstTransformPointer));

		}
		if(mask_filename)
		{
			maskReader->SetFileName(mask_filename);
			maskReader->Update();
			mask->SetImage(maskReader->GetOutput());
			mask->Update();
			registrationFactory->SetFixedImageMask(mask);
		}

		if(pointset_filename)
		{
			registrationFactory->UserOptions.LANDMARKINITIALIZE = true;
			std::ifstream pointSetFile;
			pointSetFile.open(pointset_filename);
			if(pointSetFile.fail())
			{
				std::cout << "Pointset file " << pointset_filename << " not found!" << std::endl;
				return EXIT_FAILURE;
			}
			LandmarkBasedTransformInitializerType::LandmarkPointContainer fixedPointsContainer;
			LandmarkBasedTransformInitializerType::LandmarkPointContainer movingPointsContainer;
			LandmarkBasedTransformInitializerType::LandmarkPointType fixedPoint;
			LandmarkBasedTransformInitializerType::LandmarkPointType movingPoint;
			pointSetFile >> fixedPoint;
			pointSetFile >> movingPoint;

			while( !pointSetFile.eof() )
			{
				fixedPointsContainer.push_back( fixedPoint );
				movingPointsContainer.push_back( movingPoint );
				pointSetFile >> fixedPoint;
				pointSetFile >> movingPoint;

			}

			registrationFactory->SetFixedPointContainer( fixedPointsContainer );
			registrationFactory->SetMovingPointContainer( movingPointsContainer );

		}

		registrationFactory->UserOptions.NumberOfIterations = number_of_iterations;
		registrationFactory->UserOptions.NumberOfBins = number_of_bins;
		registrationFactory->UserOptions.PixelDensity = pixel_density;
		registrationFactory->UserOptions.BSplineGridSize = grid_size;
		registrationFactory->UserOptions.PRINTRESULTS = true;
		registrationFactory->UserOptions.NumberOfThreads = number_threads;
		registrationFactory->UserOptions.MattesMutualInitializeSeed = initial_seed;
		registrationFactory->UserOptions.SHOWITERATIONSTATUS = true;
		if (!initialize_center) registrationFactory->UserOptions.INITIALIZECENTEROFF = true;
		if (!initialize_mass) registrationFactory->UserOptions.INITIALIZEMASSOFF = true;

		registrationFactory->SetFixedImage(fixedImage);
		registrationFactory->SetMovingImage(movingImage);

		std::cout << "starting the registration..." << std::endl;

		registrationFactory->StartRegistration();
		if(use_inverse) tmpTransform->SetParameters(registrationFactory->GetRegistrationObject()->GetTransform()->GetInverseTransform()->GetParameters());		
		
		if(!use_inverse) tmpConstTransformPointer = registrationFactory->GetTransform();
		
		transformMerger->push_back(tmpTransform);

	}//end repetition
	transformMerger->setTemplateImage(fixedReader->GetOutput());

	//safe the gained transform to a user specific filename
	if (out_filename) {

		if(use_inverse) transformWriter->SetInput(tmpTransform);
		if(!use_inverse) transformWriter->SetInput(tmpConstTransformPointer);
		transformWriter->SetFileName(out_filename);
		transformWriter->Update();
	}

	if (vout_filename) {
		std::cout << "creating vector deformation field..." << std::endl;
		if (repetition > 1)
		{
			transformMerger->merge();
			vectorWriter->SetInput(transformMerger->getTransform());
		}
		else
		{
			vectorWriter->SetInput(registrationFactory->GetTransformVectorField());
		}
		vectorWriter->SetFileName(vout_filename);
		vectorWriter->Update();

	}

	return 0;

}

