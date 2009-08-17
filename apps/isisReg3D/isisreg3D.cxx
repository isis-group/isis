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


//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber

//command line parser options
VString ref_filename = NULL;
VString in_filename = NULL;
VString out_filename = NULL;
VShort number_of_bins = 50;
VShort number_of_iterations = 200;
VBoolean transformVersorRigid = false;
VBoolean transformQuaternionRigid = false;
VBoolean transformEulerRigid = false;
VBoolean metricNormalizeMutualInformation = false;
VBoolean metricMattesMutualInformation = false;

static VOptionDescRec options[] = {
		{"ref", VStringRepn, 1, &ref_filename, VRequiredOpt, 0,
		"the fixed image filename" },
		{"in", VStringRepn, 1, &in_filename, VRequiredOpt, 0,
		"the moving image filename" },
		{"out", VStringRepn, 1, &out_filename, VRequiredOpt, 0,
		"the output image filename" },
		{"bins", VShortRepn, 1, &number_of_bins, VOptionalOpt, 0,
		"Number of bins used by the MattesMutualInformationMetric to calculate the image histogram"	},
		{"iter", VShortRepn, 1, &number_of_iterations, VOptionalOpt, 0,
		"Maximum number of iteration used by the optimizer" },
		{"VersorRigid", VBooleanRepn, 1, &transformVersorRigid, VOptionalOpt, 0,
		"Using a VersorRigid transform"	},
		{"QuaternionRigid", VBooleanRepn, 1, &transformQuaternionRigid, VOptionalOpt, 0,
		"Using a QuaternionRigid transform" },
		{"EulerRigid", VBooleanRepn, 1, &transformEulerRigid, VOptionalOpt, 0,
		"Using a CenteredEuler3DRigid transform" },
		{"NormalizedMutualInformation", VBooleanRepn, 1, &metricNormalizeMutualInformation, VOptionalOpt, 0,
		"Using a NormalizedMutualInformation metric" }

};

int main( int argc, char* argv[] )
{
	VParseCommand( VNumber(options), options,  & argc, argv );

	//define the standard transform type used for registration if the user has not specified it
	if (!transformVersorRigid and !transformQuaternionRigid and !transformEulerRigid)
	{
		transformVersorRigid = true;
	}
	//define the standard metric type used for registration if the user has not specified a it
	if (!metricMattesMutualInformation and !metricNormalizeMutualInformation)
	{
		metricMattesMutualInformation = true;
	}

	typedef short InputPixelType;
	typedef short OutputPixelType;
	const unsigned int Dimension = 3;

	typedef itk::Image< InputPixelType, Dimension > FixedImageType;
	typedef itk::Image< InputPixelType, Dimension > MovingImageType;
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

	typedef itk::ImageFileReader< FixedImageType > FixedImageReaderType;
	typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;
	typedef itk::ImageFileWriter< OutputImageType > WriterType;

	typedef isis::RegistrationFactory3D< FixedImageType, MovingImageType > RegistrationFactoryType;

	FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
	MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	fixedReader->SetFileName( ref_filename );
	movingReader->SetFileName( in_filename );
	writer->SetFileName( out_filename );

	fixedReader->Update();
	movingReader->Update();


	RegistrationFactoryType::Pointer registrationFactory = RegistrationFactoryType::New();
	registrationFactory->SetInterpolator( RegistrationFactoryType::Linear );


	//transform setup
	if (transformVersorRigid)
	{
		std::cout << "transform: VersorRigid3D" << std::endl;
		registrationFactory->SetTransform( RegistrationFactoryType::VersorRigid3DTransform );
	}
	if (transformQuaternionRigid)
	{
		std::cout << "transform: QuaternionRigid3D" << std::endl;
		registrationFactory->SetTransform( RegistrationFactoryType::QuaternionRigidTransform );
	}
	if (transformEulerRigid)
	{
		std::cout << "transform: CenteredEulerRigid3D" << std::endl;
		registrationFactory->SetTransform( RegistrationFactoryType::CenteredEuler3DTransform );
	}

	//metric setup
	if (metricMattesMutualInformation)
	{
		std::cout << "metric: MattesMutualInformation" << std::endl;
		registrationFactory->SetMetric( RegistrationFactoryType::MattesMutualInformation );

	}
	if (metricNormalizeMutualInformation)
	{
		std::cout << "metric: NormalizedMutualInformation" << std::endl;
		registrationFactory->SetMetric( RegistrationFactoryType::NormalizedMutualInformation );
	}


	registrationFactory->SetOptimizer( RegistrationFactoryType::VersorRigidOptimizer );
	registrationFactory->UserOptions.NumberOfIterations = number_of_iterations;
	registrationFactory->UserOptions.NumberOfBins = number_of_bins;
	registrationFactory->UserOptions.PRINTRESULTS = true;

	registrationFactory->SetFixedImage( fixedReader->GetOutput() );
	registrationFactory->SetMovingImage( movingReader->GetOutput() );
	registrationFactory->StartRegistration();
	writer->SetInput( registrationFactory->GetRegisteredImage() );
	writer->Update();














	return 0;


}
