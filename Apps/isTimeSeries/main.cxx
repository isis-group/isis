/*
 * main.cxx
 *
 *  Created on: May 14, 2009
 *      Author: tuerke
 */

//programm related stuff
#include "timeStepExtractorFilter.h"

#include "RegistrationInterface.h"


#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkExtractImageFilter.h"

//registration related files




int main( int argc, char** argv)
{
	typedef short PixelType;
	const unsigned int inputDimension = 4;
	const unsigned int outputDimension = 3;
	typedef itk::Image< PixelType, inputDimension > InputImageType;
	typedef itk::Image< PixelType, outputDimension > OutputImageType;

	typedef itk::Image< PixelType, 3 > FixedImageType;
	typedef itk::Image< PixelType, 3 > MovingImageType;

	typedef TimeStepExtractorFilter< InputImageType, OutputImageType > TimeStepExtractorFilterType;

	typedef RegistrationInterface< FixedImageType, MovingImageType > RegistrationInterfaceType;








	typedef itk::ImageFileReader< InputImageType > ReaderType;
	typedef itk::ImageFileWriter< OutputImageType > WriterType;





	TimeStepExtractorFilterType* extractorFilter = new TimeStepExtractorFilterType;

	RegistrationInterfaceType* registrator = new RegistrationInterfaceType;





	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();

	InputImageType::Pointer inputImage = InputImageType::New();
	OutputImageType::Pointer outputImage = OutputImageType::New();

	FixedImageType::Pointer fixedImage = FixedImageType::New();
	MovingImageType::Pointer movingImage = MovingImageType::New();

	reader->SetFileName( argv[1] );
	inputImage = reader->GetOutput();
	inputImage->Update();
	//image->Print(std::cout);



	extractorFilter->SetSliceNumber( atoi(argv[2]) );
	extractorFilter->SetInputImage( inputImage );
	extractorFilter->Start();

	outputImage = extractorFilter->GetOutputImage();
	writer->SetFileName( argv[3] );
	writer->SetInput( outputImage );
	writer->Update();

	std::cout << "Number of time steps: " << extractorFilter->GetNumberOfTimeSteps() << std::endl;

	registrator->SetFixedImage( fixedImage );
	registrator->SetMovingImage( movingImage );
	registrator->SetMetric( RegistrationInterfaceType::MEANSQUARE );
	registrator->SetRegistrationMethod( RegistrationInterfaceType::TRANSLATION );
	registrator->StartRegistration();



	return 0;

}

