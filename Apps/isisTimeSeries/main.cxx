/*
 * main.cxx
 *
 *  Created on: May 14, 2009
 *      Author: tuerke
 */

//programm related stuff
#include "isisTimeStepExtractionFilter.h"



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

	typedef isis::TimeStepExtractionFilter< InputImageType, OutputImageType >
		TimeStepExtractionFilterType;





	typedef itk::ImageFileReader< InputImageType > ReaderType;
	typedef itk::ImageFileWriter< OutputImageType > WriterType;

	ReaderType::Pointer reader = ReaderType::New();
	WriterType::Pointer writer = WriterType::New();




	TimeStepExtractionFilterType::Pointer extractionFilter =
		TimeStepExtractionFilterType::New();

	reader->SetFileName( argv[1] );
	reader->Update();

	extractionFilter->SetInput( reader->GetOutput() );

	extractionFilter->SetRequestedTimeStep( atoi(argv[2]) );

	extractionFilter->Update();

	writer->SetFileName( argv[3] );
	writer->SetInput( extractionFilter->GetOutput() );
	writer->Update();




	return 0;

}

