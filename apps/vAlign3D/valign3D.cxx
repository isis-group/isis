/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 *****************************************************************/


#include <itkWarpImageFilter.h>
#include <itkImageMaskSpatialObject.h>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImage.h>

#include <itkTransformFileWriter.h>
#include <itkTransformFileReader.h>

#include <itkLandmarkBasedTransformInitializer.h>
#include <itkVersorRigid3DTransform.h>

#include <itkMedianImageFilter.h>
#include <itkDiscreteGaussianImageFilter.h>
#include <itkRecursiveGaussianImageFilter.h>

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/progress.hpp>

//via command parser include
#include <viaio/option.h>
#include <viaio/mu.h> //this is required for VNumber

//isis includes
#include "CoreUtils/log.hpp"
#include "DataStorage/io_factory.hpp"
#include "DataStorage/image.hpp"
#include "CoreUtils/application.hpp"
#include "ExternalLibraryAdapter/itkAdapter.hpp"

#include "extRegistration/isisRegistrationFactory3D.hpp"
#include "extITK/isisIterationObserver.hpp"

VDictEntry TYPMetric[] = { {"MattesMutualInformation", 0}, {"MutualInformationHistogram", 1}, {"NormalizedCorrelation",
		2
	}, {"MeanSquare", 3}, {NULL}
};

VDictEntry TYPTransform[] = { {"VersorRigid", 0}, {"Affine", 1}, {"BSplineDeformable", 2}, {"Translation", 3}, {"Scale", 4}, {"CenteredAffine", 5}, {NULL}};

VDictEntry TYPInterpolator[] = { {"Linear", 0}, {"BSpline", 1}, {"NearestNeighbor", 2}, {NULL}};

VDictEntry TYPOptimizer[] = { {"VersorRigid", 0}, {"RegularStepGradientDescent", 1}, {"LBFGSB", 2}, {"Amoeba", 3}, {
		"Powell", 4
	}, {NULL}
};

//command line parser options
static VString ref_filename = NULL;
static VString in_filename = NULL;
static VString out_filename = NULL;
static VString vout_filename = NULL;
static VString pointset_filename = NULL;
static VString transform_filename_in = NULL;
static VShort number_of_bins = 50;
static VArgVector number_of_iterations;
static VFloat pixel_density = 0.01;
static VArgVector grid_size;
static VArgVector transformType;
static VArgVector interpolatorType;
static VArgVector optimizerType;
static VArgVector metricType;
static VBoolean in_found, ref_found, pointset_found;
static VShort number_threads = 1;
static VShort initial_seed = 1;
static VBoolean initialize_center = false;
static VBoolean initialize_mass = false;
static VString mask_filename = NULL;
static VFloat smooth = 0;
static VBoolean use_inverse = false;
static VFloat coarse_factor = 1;
static VFloat bspline_bound = 100.0;
static VBoolean verbose = true;
static VBoolean fmri = false;

static VOptionDescRec
options[] = {
	//required inputs
	{"ref", VStringRepn, 1, &ref_filename, &ref_found, 0, "the fixed image filename"},
	{"in", VStringRepn, 1, &in_filename, &in_found, 0, "the moving image filename"},

	//non-required inputs
	{"mask", VStringRepn, 1, &mask_filename, VOptionalOpt, 0, "the mask filename"},
	{"pointset", VStringRepn, 1, &pointset_filename, &pointset_found, 0, "the pointset filename"},
	{"itktrans", VStringRepn, 1, &out_filename, VOptionalOpt, 0, "the itk output transform filename"},
	{"trans", VStringRepn, 1, &vout_filename, VOptionalOpt, 0, "the output vector image filename. Has to be of type nifti."},
	{
		"tin", VStringRepn, 1, &transform_filename_in, VOptionalOpt, 0,
		"filename of the transform used as an initial transform"
	},
	//parameter inputs
	{
		"bins", VShortRepn, 1, &number_of_bins, VOptionalOpt, 0,
		"Number of bins used by the MattesMutualInformationMetric to calculate the image histogram"
	},
	{
		"iter", VShortRepn, 0, ( VPointer ) &number_of_iterations, VOptionalOpt, 0,
		"Maximum number of iteration used by the optimizer"
	},
	{
		"seed", VShortRepn, 1, &initial_seed, VOptionalOpt, 0,
		"The initialize seed for the MattesMutualInformationMetric"
	},

	{
		"pd",
		VFloatRepn,
		1,
		&pixel_density,
		VOptionalOpt,
		0,
		"The density of pixels the metric uses. 1 denotes the metric uses all pixels. Has to be > 0. Only operative with a MattesMutualInformation metric"
	},

	{"j", VShortRepn, 1, &number_threads, VOptionalOpt, 0, "Number of threads used for the registration"},
	{"cf", VFloatRepn, 1, &coarse_factor, VOptionalOpt, 0, "Coarse factor. Multiple of the max and min step length of the optimizer. Standard is 1"},
	{"bound", VFloatRepn, 1, &bspline_bound, VOptionalOpt, 0, "max/min value of the bepline deformation."},
	{
		"gridSize", VShortRepn, 0, ( VPointer ) &grid_size, VOptionalOpt, 0,
		"Grid size used for the BSplineDeformable transform."
	},

	{
		"prealign_center", VBooleanRepn, 1, &initialize_center, VOptionalOpt, 0,
		"Using an initializer to align the image centers"
	},
	{
		"prealign_mass", VBooleanRepn, 1, &initialize_mass, VOptionalOpt, 0,
		"Using an initializer to align the center of mass"
	},
	{"verbose", VBooleanRepn, 1, &verbose, VOptionalOpt, 0, "printing the optimizer values of each iteration"},
	{"fmri", VBooleanRepn, 1, &fmri, VOptionalOpt, 0, "Input image file contains functional data"},
	{"smooth", VFloatRepn, 1, &smooth, VOptionalOpt, 0, "Applying a smoothing filter to the fixed and moving image before the registration process"},
	{"get_inverse", VBooleanRepn, 1, &use_inverse, VOptionalOpt, 0, "Getting the inverse transform"},

	//component inputs
	{"metric", VShortRepn, 0, ( VPointer ) &metricType, VOptionalOpt, TYPMetric, "Type of the metric"}, {
		"transform", VShortRepn, 0, ( VPointer ) &transformType, VOptionalOpt, TYPTransform,
		"Type of the transform"
	}, {"interpolator", VShortRepn, 0, ( VPointer ) &interpolatorType, VOptionalOpt,
		TYPInterpolator, "Type of interpolator"
	   }, {"optimizer", VShortRepn, 0, ( VPointer ) &optimizerType,
		   VOptionalOpt, TYPOptimizer, "Type of optimizer"
		  }

};

// This is the main function
int main(
	int argc, char *argv[] )
{
	// show revision information string constant
	std::cout << "Core Version: " << isis::util::Application::getCoreVersion() << std::endl;
	ENABLE_LOG( isis::DataDebug, isis::util::DefaultMsgPrint, isis::error );
	ENABLE_LOG( isis::DataLog, isis::util::DefaultMsgPrint, isis::error );

	// DANGER! Kids don't try this at home! VParseCommand modifies the values of argc and argv!!!
	if ( !VParseCommand( VNumber( options ), options, &argc, argv ) || !VIdentifyFiles( VNumber( options ), options, "in",
			&argc, argv, 0 ) ) {
		VReportUsage( argv[0], VNumber( options ), options, NULL );
		exit( 1 );
	}

	// VParseCommand reduces the argv vector to the name of the program and  unknown command line parameters.
	if ( argc > 1 ) {
		VReportBadArgs( argc, argv );
		VReportUsage( argv[0], VNumber( options ), options, NULL );
		exit( 1 );
	}

	typedef unsigned char MaskPixelType;
	typedef signed short InputPixelType;
	typedef signed short OutputPixelType;
	const unsigned int Dimension = 3;
	VShort transform;
	VShort optimizer;
	VShort interpolator;
	VShort metric;
	VShort niteration;
	VShort gridSize;
	typedef itk::Image<InputPixelType, Dimension> FixedImageType;
	typedef itk::Image<InputPixelType, Dimension> MovingImageType;
	typedef itk::Image<MaskPixelType, Dimension> MaskImageType;
	typedef itk::ImageMaskSpatialObject<Dimension> ImageMaskSpatialObjectType;
	typedef itk::ImageFileReader<MaskImageType> MaskImageReaderType;
	typedef isis::registration::RegistrationFactory3D<FixedImageType, MovingImageType> RegistrationFactoryType;
	typedef itk::Vector<float, Dimension> VectorType;
	typedef itk::Image<VectorType, Dimension> DeformationFieldType;
	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::LandmarkBasedTransformInitializer<VersorRigid3DTransformType, FixedImageType, MovingImageType>
	RigidLandmarkBasedTransformInitializerType;
	typedef itk::PointSet<float, Dimension> PointSetType;
	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::LandmarkBasedTransformInitializer<VersorRigid3DTransformType, FixedImageType, MovingImageType>
	LandmarkBasedTransformInitializerType;
	typedef itk::ImageFileWriter<DeformationFieldType> VectorWriterType;
	typedef itk::ImageFileWriter<MovingImageType> WriterType;
	const itk::TransformBase *tmpConstTransformPointer;
	typedef itk::TransformBase *TransformBasePointerType;
	typedef itk::MedianImageFilter<FixedImageType, FixedImageType> FixedFilterType;
	typedef itk::MedianImageFilter<MovingImageType, MovingImageType> MovingFilterType;
	typedef itk::RecursiveGaussianImageFilter<FixedImageType, FixedImageType> GaussianFilterType;
	MaskImageReaderType::Pointer maskReader = MaskImageReaderType::New();
	itk::AffineTransform<double, Dimension>::Pointer tmpTransform = itk::AffineTransform<double, Dimension>::New();
	itk::TransformFileWriter::Pointer transformWriter = itk::TransformFileWriter::New();
	VectorWriterType::Pointer vectorWriter = VectorWriterType::New();
	WriterType::Pointer writer = WriterType::New();
	itk::TransformFileReader::Pointer transformReader = itk::TransformFileReader::New();
	ImageMaskSpatialObjectType::Pointer mask = ImageMaskSpatialObjectType::New();
	FixedImageType::Pointer fixedImage = FixedImageType::New();
	MovingImageType::Pointer movingImage = MovingImageType::New();
	FixedFilterType::Pointer fixedFilter = FixedFilterType::New();
	MovingFilterType::Pointer movingFilter = MovingFilterType::New();
	tmpConstTransformPointer = 0;
	isis::adapter::itkAdapter *fixedAdapter = new isis::adapter::itkAdapter;
	isis::adapter::itkAdapter *movingAdapter = new isis::adapter::itkAdapter;
	isis::data::ImageList refList = isis::data::IOFactory::load( ref_filename, "", "" );
	isis::data::ImageList inList;
	if( fmri ) {
		inList = isis::data::IOFactory::load( in_filename, "", "functional");
	}
	if( not fmri ) {
		inList = isis::data::IOFactory::load( in_filename, "", "" );
	}
	LOG_IF( refList.empty(), isis::DataLog, isis::error ) << "Reference image is empty!";
	LOG_IF( inList.empty(), isis::DataLog, isis::error ) << "Input image is empty!";


	// TODO DEBUG
	// use makeItkImageObject with "false" -> no ITK nifti default transformation
	if ( !smooth ) {
		fixedImage = fixedAdapter->makeItkImageObject<FixedImageType>( refList.front() );
		movingImage = movingAdapter->makeItkImageObject<MovingImageType>( inList.front() );
		writer->SetFileName("test.nii");
		writer->SetInput(movingImage);
		writer->Update();
		// TODO DEBUG
		//      std::cout << "********** Fixed Image **********" << std::endl;
		//      std::cout << fixedImage->GetDirection();
		//      std::cout << "index origin: ";
		//      std::cout << fixedImage->GetOrigin() << std::endl;
		//
		//      FixedImageType::PointType fpoint;
		//      FixedImageType::SizeType fsize = fixedImage->GetLargestPossibleRegion().GetSize();
		//      FixedImageType::IndexType findex = {{fsize[0],fsize[1],fsize[2]}};
		//      fixedImage->TransformIndexToPhysicalPoint(findex,fpoint);
		//      std::cout << "diagonal point:" << fpoint << std::endl;
		//      std::cout << "size: " << fsize << std::endl;
		//      std::cout << "spacing: " << fixedImage->GetSpacing() << std::endl << std::endl;
		//
		//      std::cout << "********** Moving Image **********" << std::endl;
		//      std::cout << movingImage->GetDirection();
		//      std::cout << "index origin: ";
		//      std::cout << movingImage->GetOrigin() << std::endl;
		//
		//      MovingImageType::PointType mpoint;
		//      FixedImageType::SizeType msize = movingImage->GetLargestPossibleRegion().GetSize();
		//      FixedImageType::IndexType mindex = {{msize[0],msize[1],msize[2]}};
		//      movingImage->TransformIndexToPhysicalPoint(mindex,mpoint);
		//      std::cout << "diagonal point:" << mpoint << std::endl;
		//      std::cout << "size: " << msize << std::endl;
		//      std::cout << "spacing: " << movingImage->GetSpacing() << std::endl;
	}

	if ( smooth ) {
		GaussianFilterType::Pointer fixedGaussianFilterX = GaussianFilterType::New();
		GaussianFilterType::Pointer fixedGaussianFilterY = GaussianFilterType::New();
		GaussianFilterType::Pointer fixedGaussianFilterZ = GaussianFilterType::New();
		fixedGaussianFilterX->SetNumberOfThreads( number_threads );
		fixedGaussianFilterY->SetNumberOfThreads( number_threads );
		fixedGaussianFilterZ->SetNumberOfThreads( number_threads );
		fixedGaussianFilterX->SetInput( fixedAdapter->makeItkImageObject<FixedImageType>( refList.front() ) );
		fixedGaussianFilterY->SetInput( fixedGaussianFilterX->GetOutput() );
		fixedGaussianFilterZ->SetInput( fixedGaussianFilterY->GetOutput() );
		fixedGaussianFilterX->SetDirection( 0 );
		fixedGaussianFilterY->SetDirection( 1 );
		fixedGaussianFilterZ->SetDirection( 2 );
		fixedGaussianFilterX->SetOrder( GaussianFilterType::ZeroOrder );
		fixedGaussianFilterY->SetOrder( GaussianFilterType::ZeroOrder );
		fixedGaussianFilterZ->SetOrder( GaussianFilterType::ZeroOrder );
		fixedGaussianFilterX->SetNormalizeAcrossScale( false );
		fixedGaussianFilterY->SetNormalizeAcrossScale( false );
		fixedGaussianFilterZ->SetNormalizeAcrossScale( false );
		fixedGaussianFilterX->SetSigma( smooth );
		fixedGaussianFilterY->SetSigma( smooth );
		fixedGaussianFilterZ->SetSigma( smooth );
		std::cout << "smoothing the fixed image..." << std::endl;
		fixedGaussianFilterZ->Update();
		fixedImage = fixedGaussianFilterZ->GetOutput();
		GaussianFilterType::Pointer movingGaussianFilterX = GaussianFilterType::New();
		GaussianFilterType::Pointer movingGaussianFilterY = GaussianFilterType::New();
		GaussianFilterType::Pointer movingGaussianFilterZ = GaussianFilterType::New();
		movingGaussianFilterX->SetNumberOfThreads( number_threads );
		movingGaussianFilterY->SetNumberOfThreads( number_threads );
		movingGaussianFilterZ->SetNumberOfThreads( number_threads );
		movingGaussianFilterX->SetInput( movingAdapter->makeItkImageObject<MovingImageType>( inList.front() ) );
		movingGaussianFilterY->SetInput( movingGaussianFilterX->GetOutput() );
		movingGaussianFilterZ->SetInput( movingGaussianFilterY->GetOutput() );
		movingGaussianFilterX->SetDirection( 0 );
		movingGaussianFilterY->SetDirection( 1 );
		movingGaussianFilterZ->SetDirection( 2 );
		movingGaussianFilterX->SetOrder( GaussianFilterType::ZeroOrder );
		movingGaussianFilterY->SetOrder( GaussianFilterType::ZeroOrder );
		movingGaussianFilterZ->SetOrder( GaussianFilterType::ZeroOrder );
		movingGaussianFilterX->SetNormalizeAcrossScale( false );
		movingGaussianFilterY->SetNormalizeAcrossScale( false );
		movingGaussianFilterZ->SetNormalizeAcrossScale( false );
		movingGaussianFilterX->SetSigma( smooth );
		movingGaussianFilterY->SetSigma( smooth );
		movingGaussianFilterZ->SetSigma( smooth );
		std::cout << "smoothing the moving image..." << std::endl;
		movingGaussianFilterZ->Update();
		movingImage = movingGaussianFilterZ->GetOutput();
	}

	RegistrationFactoryType::Pointer registrationFactory = RegistrationFactoryType::New();
	//analyse transform vector
	//transform is the master for determining the number of repetitions
	int repetition = transformType.number;
	int bsplineCounter = 0;

	if ( !repetition )
		repetition = 1;

	//analyse optimizer vector
	boost::progress_timer time_used;

	for ( int counter = 0; counter < repetition; counter++ ) {
		//transform is the master for determining the number of repetitions
		if ( transformType.number ) {
			transform = ( ( VShort * ) transformType.vector )[counter];
		} else {
			transform = 0;
		}

		if ( ( counter + 1 ) <= optimizerType.number and optimizerType.number ) {
			optimizer = ( ( VShort * ) optimizerType.vector )[counter];
		} else if ( ( counter + 1 ) > optimizerType.number and optimizerType.number ) {
			optimizer = ( ( VShort * ) optimizerType.vector )[optimizerType.number-1];
		} else {
			optimizer = 0;
		}

		if ( ( counter + 1 ) <= metricType.number and metricType.number ) {
			metric = ( ( VShort * ) metricType.vector )[counter];
		} else if ( ( counter + 1 ) > metricType.number and metricType.number ) {
			metric = ( ( VShort * ) metricType.vector )[metricType.number-1];
		} else {
			metric = 0;
		}

		if ( ( counter + 1 ) <= interpolatorType.number and interpolatorType.number ) {
			interpolator = ( ( VShort * ) interpolatorType.vector )[counter];
		} else if ( ( counter + 1 ) > interpolatorType.number and interpolatorType.number ) {
			interpolator = ( ( VShort * ) interpolatorType.vector )[interpolatorType.number-1];
		} else {
			interpolator = 0;
		}

		if ( ( counter + 1 ) <= number_of_iterations.number and number_of_iterations.number ) {
			niteration = ( ( VShort * ) number_of_iterations.vector )[counter];
		} else if ( ( counter + 1 ) > number_of_iterations.number and number_of_iterations.number ) {
			niteration = ( ( VShort * ) number_of_iterations.vector )[number_of_iterations.number-1];
		} else {
			niteration = 500;
		}

		if ( ( bsplineCounter + 1 ) <= grid_size.number and grid_size.number ) {
			gridSize = ( ( VShort * ) grid_size.vector )[bsplineCounter];
		} else if ( ( bsplineCounter + 1 ) > grid_size.number and grid_size.number ) {
			gridSize = ( ( VShort * ) grid_size.vector )[grid_size.number-1];
		} else {
			gridSize = 5;
		}

		if ( transform == 2 ) {
			bsplineCounter++;
		}

		std::cout << std::endl << "setting up the registration object..." << std::endl;
		registrationFactory->Reset();

		//check pixel density
		if ( pixel_density <= 0 ) {
			std::cerr << "wrong pixel density...set to 0.01" << std::endl;
			pixel_density = 0.01;
		}

		if ( pixel_density >= 1 ) {
			std::cerr << "metric uses all pixels" << std::endl;
		}

		//check grid size
		if ( gridSize <= 4 ) {
			std::cerr << "\ngrid size has to be bigger than 4...setting grid size to 5\n" << std::endl;
			gridSize = 5;
		}

		//check combinations of components
		//      if ( optimizer != 0 and transform == 0 ) {
		//          std::cerr
		//              << "\nInappropriate combination of transform and optimizer! Setting optimizer to VersorRigidOptimizer.\n"
		//              << std::endl;
		//          optimizer = 0;
		//      }

		if ( transform == 2 and optimizer != 2 ) {
			std::cerr << "\nIt is recommended using the BSpline transform in connection with the LBFGSB optimizer!\n"
					  << std::endl;
		}

		if ( transform != 0 and optimizer == 0 ) {
			std::cerr << "\nInappropriate combination of transform and optimizer! Setting optimizer to RegularStepGradientDescentOptimizer.\n"
					  << std::endl;
			optimizer = 1;
		}

		//transform setup
		std::cout << "used transform: " << TYPTransform[transform].keyword << std::endl;

		switch ( transform ) {
		case 0:
			registrationFactory->SetTransform( RegistrationFactoryType::VersorRigid3DTransform );
			break;
		case 1:
			registrationFactory->SetTransform( RegistrationFactoryType::AffineTransform );
			break;
		case 2:
			registrationFactory->SetTransform( RegistrationFactoryType::BSplineDeformableTransform );
			break;
		case 3:
			registrationFactory->SetTransform( RegistrationFactoryType::TranslationTransform );
			break;
		case 4:
			registrationFactory->SetTransform( RegistrationFactoryType::ScaleTransform );
			break;
		case 5:
			registrationFactory->SetTransform( RegistrationFactoryType::CenteredAffineTransform );
			break;
		}

		//metric setup
		std::cout << "used metric: " << TYPMetric[metric].keyword << std::endl;

		switch ( metric ) {
		case 0:
			registrationFactory->SetMetric( RegistrationFactoryType::MattesMutualInformationMetric );
			break;
		case 1:
			registrationFactory->SetMetric( RegistrationFactoryType::MutualInformationHistogramMetric );
			break;
		case 2:
			registrationFactory->SetMetric( RegistrationFactoryType::NormalizedCorrelationMetric );
			break;
		case 3:
			registrationFactory->SetMetric( RegistrationFactoryType::MeanSquareMetric );
			break;
		}

		//interpolator setup
		std::cout << "used interpolator: " << TYPInterpolator[interpolator].keyword << std::endl;

		switch ( interpolator ) {
		case 0:
			registrationFactory->SetInterpolator( RegistrationFactoryType::LinearInterpolator );
			break;
		case 1:
			registrationFactory->SetInterpolator( RegistrationFactoryType::BSplineInterpolator );
			break;
		case 2:
			registrationFactory->SetInterpolator( RegistrationFactoryType::NearestNeighborInterpolator );
			break;
		}

		//optimizer setup
		std::cout << "used optimizer: " << TYPOptimizer[optimizer].keyword << std::endl;

		switch ( optimizer ) {
		case 0:
			registrationFactory->SetOptimizer( RegistrationFactoryType::VersorRigidOptimizer );
			break;
		case 1:
			registrationFactory->SetOptimizer( RegistrationFactoryType::RegularStepGradientDescentOptimizer );
			break;
		case 2:
			registrationFactory->SetOptimizer( RegistrationFactoryType::LBFGSBOptimizer );
			break;
		case 3:
			registrationFactory->SetOptimizer( RegistrationFactoryType::AmoebaOptimizer );
			break;
		case 4:
			registrationFactory->SetOptimizer( RegistrationFactoryType::PowellOptimizer );
			break;
		}

		if ( transform_filename_in and counter == 0 ) {
			initialize_mass = false;
			initialize_center = false;
			transformReader->SetFileName( transform_filename_in );
			transformReader->Update();
			itk::TransformFileReader::TransformListType *transformList = transformReader->GetTransformList();
			itk::TransformFileReader::TransformListType::const_iterator ti;
			ti = transformList->begin();
			registrationFactory->SetInitialTransform( ( *ti ).GetPointer() );
		}

		if ( counter != 0 ) {
			registrationFactory->UserOptions.INITIALIZECENTEROFF = true;
			registrationFactory->UserOptions.INITIALIZEMASSOFF = true;
			registrationFactory->SetInitialTransform( const_cast<TransformBasePointerType> ( tmpConstTransformPointer ) );
		}

		if ( mask_filename ) {
			maskReader->SetFileName( mask_filename );
			maskReader->Update();
			mask->SetImage( maskReader->GetOutput() );
			mask->Update();
			registrationFactory->SetFixedImageMask( mask );
		}

		if ( pointset_filename ) {
			registrationFactory->UserOptions.LANDMARKINITIALIZE = true;
			std::ifstream pointSetFile;
			pointSetFile.open( pointset_filename );

			if ( pointSetFile.fail() ) {
				std::cout << "Pointset file " << pointset_filename << " not found!" << std::endl;
				return EXIT_FAILURE;
			}

			LandmarkBasedTransformInitializerType::LandmarkPointContainer fixedPointsContainer;
			LandmarkBasedTransformInitializerType::LandmarkPointContainer movingPointsContainer;
			LandmarkBasedTransformInitializerType::LandmarkPointType fixedPoint;
			LandmarkBasedTransformInitializerType::LandmarkPointType movingPoint;
			pointSetFile >> fixedPoint;
			pointSetFile >> movingPoint;

			while ( !pointSetFile.eof() ) {
				fixedPointsContainer.push_back( fixedPoint );
				movingPointsContainer.push_back( movingPoint );
				pointSetFile >> fixedPoint;
				pointSetFile >> movingPoint;
			}

			registrationFactory->SetFixedPointContainer( fixedPointsContainer );
			registrationFactory->SetMovingPointContainer( movingPointsContainer );
		}

		registrationFactory->UserOptions.CoarseFactor = coarse_factor;
		registrationFactory->UserOptions.BSplineBound = bspline_bound;
		registrationFactory->UserOptions.NumberOfIterations = niteration;
		registrationFactory->UserOptions.NumberOfBins = number_of_bins;
		registrationFactory->UserOptions.PixelDensity = pixel_density;
		registrationFactory->UserOptions.BSplineGridSize = gridSize;

		if ( verbose ) {
			registrationFactory->UserOptions.SHOWITERATIONSTATUS = true;
		} else {
			registrationFactory->UserOptions.SHOWITERATIONSTATUS = false;
		}

		registrationFactory->UserOptions.NumberOfThreads = number_threads;
		registrationFactory->UserOptions.MattesMutualInitializeSeed = initial_seed;
		registrationFactory->UserOptions.PRINTRESULTS = true;

		if ( !initialize_center ) registrationFactory->UserOptions.INITIALIZECENTEROFF = true;

		if ( !initialize_mass ) registrationFactory->UserOptions.INITIALIZEMASSOFF = true;

		registrationFactory->SetFixedImage( fixedImage );
		registrationFactory->SetMovingImage( movingImage );
		std::cout << "starting the registration..." << std::endl;
		registrationFactory->StartRegistration();

		if ( use_inverse ) {
			tmpTransform->SetParameters( registrationFactory->GetRegistrationObject()->GetTransform()->GetInverseTransform()->GetParameters() );
		}

		if ( !use_inverse ) {
			tmpConstTransformPointer = registrationFactory->GetTransform();
		}

		//transformMerger->push_back(tmpTransform);
	}//end repetition

	//safe the gained transform to a user specific filename
	if ( out_filename ) {
		if ( use_inverse ) transformWriter->SetInput( tmpTransform );

		if ( !use_inverse ) transformWriter->SetInput( tmpConstTransformPointer );

		transformWriter->SetFileName( out_filename );
		transformWriter->Update();
	}

	if ( vout_filename ) {
		std::cout << "creating vector deformation field..." << std::endl;
		vectorWriter->SetInput( registrationFactory->GetTransformVectorField() );
		vectorWriter->SetFileName( vout_filename );
		vectorWriter->Update();
	}

	return 0;
}

