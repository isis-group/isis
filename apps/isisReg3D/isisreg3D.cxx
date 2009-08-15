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
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber

//command line parser options
VString ref_filename = NULL;
VString in_filename = NULL;
VString out_filename = NULL;
VShort number_of_bins = 50;
VShort number_of_iterations = 200;
VBoolean VersorRigid = false;
VBoolean QuaternionRigid = false;
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
		{"VersorRigid", VBooleanRepn, 1, &VersorRigid, VOptionalOpt, 0,
		"Using a VersorRigid transform"	},
		{"QuaternionRigid", VBooleanRepn, 1, &QuaternionRigid, VOptionalOpt, 0,
		"Using a QuaternionRigid transform" }


};

int main( int argc, char* argv[] )
{
	VParseCommand( VNumber(options), options,  & argc, argv );

	//define the standard transform type used for registration if the user has not specified any of them
	if (!VersorRigid and !QuaternionRigid)
	{
		VersorRigid = true;
	}

	typedef float InputPixelType;
	typedef short OutputPixelType;
	const unsigned int Dimension = 3;

	typedef itk::Image< InputPixelType, Dimension > FixedImageType;
	typedef itk::Image< InputPixelType, Dimension > MovingImageType;
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

	typedef itk::ImageFileReader< FixedImageType > FixedImageReaderType;
	typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;
	typedef itk::ImageFileWriter< OutputImageType > WriterType;

	typedef isis::RegistrationFactory3D< FixedImageType, MovingImageType > RegistrationFactoryType;

	typedef itk::ResampleImageFilter< MovingImageType, FixedImageType > ResampleFilterType;
	typedef itk::CastImageFilter< FixedImageType, OutputImageType > CasterType;


	FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
	MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	ResampleFilterType::Pointer resampler = ResampleFilterType::New();
	CasterType::Pointer caster = CasterType::New();

	fixedReader->SetFileName( ref_filename );
	movingReader->SetFileName( in_filename );
	writer->SetFileName( out_filename );

	fixedReader->Update();
	movingReader->Update();


	RegistrationFactoryType::Pointer registrationFactory = RegistrationFactoryType::New();
	registrationFactory->SetInterpolator( RegistrationFactoryType::Linear );

	if (VersorRigid)
	{
		std::cout << "transform: VersorRigid3D" << std::endl;
		registrationFactory->SetTransform( RegistrationFactoryType::VersorRigid3DTransform );
	}
	if (QuaternionRigid)
	{
		std::cout << "transform: QuaternionRigid3D" << std::endl;
		registrationFactory->SetTransform( RegistrationFactoryType::QuaternionRigidTransform );
	}

	registrationFactory->SetOptimizer( RegistrationFactoryType::RegularStepGradientDescentOptimizer );
	registrationFactory->UserOptions.NumberOfIterations = number_of_iterations;
	registrationFactory->UserOptions.NumberOfBins = number_of_bins;
	registrationFactory->UserOptions.PRINTRESULTS = true;
	registrationFactory->SetMetric( RegistrationFactoryType::MattesMutualInformation );

	registrationFactory->SetFixedImage( fixedReader->GetOutput() );
	registrationFactory->SetMovingImage( movingReader->GetOutput() );
	registrationFactory->StartRegistration();


	//resample
	resampler->SetTransform( registrationFactory->GetTransform() );
	resampler->SetInput( movingReader->GetOutput() );
	resampler->SetOutputOrigin( fixedReader->GetOutput()->GetOrigin() );
	resampler->SetOutputDirection( fixedReader->GetOutput()->GetDirection() );
	resampler->SetSize( fixedReader->GetOutput()->GetLargestPossibleRegion().GetSize() );
	resampler->SetOutputSpacing( fixedReader->GetOutput()->GetSpacing() );
	resampler->SetDefaultPixelValue( 0 );
	caster->SetInput( resampler->GetOutput() );


	writer->SetInput( caster->GetOutput() );
	writer->Update();














	return 0;


}
