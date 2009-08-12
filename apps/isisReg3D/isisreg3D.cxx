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





int main( int argc, char** argv )
{

	typedef float InputPixelType;
	typedef short OutputPixelType;
	const unsigned int Dimension = 3;

	typedef itk::Image< InputPixelType, Dimension > FixedImageType;
	typedef itk::Image< InputPixelType, Dimension > MovingImageType;
	typedef itk::Image< OutputPixelType, Dimension > OutputImageType;

	typedef itk::ImageFileReader< FixedImageType > FixedImageReaderType;
	typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;
	typedef itk::ImageFileWriter< OutputImageType > WriterType;

	typedef isis::RegistrationFactory< FixedImageType, MovingImageType > RegistrationFactoryType;

	typedef itk::ResampleImageFilter< MovingImageType, FixedImageType > ResampleFilterType;
	typedef itk::CastImageFilter< FixedImageType, OutputImageType > CasterType;


	FixedImageReaderType::Pointer fixedReader = FixedImageReaderType::New();
	MovingImageReaderType::Pointer movingReader = MovingImageReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	ResampleFilterType::Pointer resampler = ResampleFilterType::New();
	CasterType::Pointer caster = CasterType::New();

	fixedReader->SetFileName( argv[1] );
	movingReader->SetFileName( argv[2] );
	writer->SetFileName( argv[3] );

	fixedReader->Update();
	movingReader->Update();


	RegistrationFactoryType::Pointer registrationFactory = RegistrationFactoryType::New();

	registrationFactory->SetInterpolator( RegistrationFactoryType::Linear );
	registrationFactory->SetTransform( RegistrationFactoryType::VersorRigid3DTransform );
	registrationFactory->SetOptimizer( RegistrationFactoryType:: );
	registrationFactory->UserOptions.NumberOfIterations = 300;
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
