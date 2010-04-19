/*
 * isisCheckerboard3D.cxx
 *
 *  Created on: November 4, 2009
 *      Author: tuerke
 */


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkCheckerBoardImageFilter.h"
//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber


static VArgVector in_filename;
static VString out_filename = 0;
static VShort number_patterns = 5;
static VBoolean in_found, out_found;

static VOptionDescRec
options[] = {
	//requiered inputs
	{"in", VStringRepn, 0, ( VPointer ) &in_filename, &in_found, 0, "the input image filenames"}, {"out", VStringRepn, 1,
			&out_filename, &out_found, 0, "the output image filename"}, {"patterns", VShortRepn, 1, &number_patterns, VOptionalOpt, 0, "Number of patterns"}
};

int main( int argc, char* argv[] )
{
	// show revision information string constant
	std::cout << "Revision: " << _SVN_REVISION << std::endl;

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

	const unsigned int Dimension = 3;

	typedef short PixelType;

	typedef itk::Image<PixelType, Dimension> ImageType;

	typedef itk::ImageFileReader<ImageType> ImageFileReaderType;

	typedef itk::ImageFileWriter<ImageType> ImageFileWriterType;

	typedef itk::CheckerBoardImageFilter<ImageType> CheckerBoardImageFilterType;

	ImageFileReaderType::Pointer reader1 = ImageFileReaderType::New();

	ImageFileReaderType::Pointer reader2 = ImageFileReaderType::New();

	ImageFileWriterType::Pointer writer = ImageFileWriterType::New();

	CheckerBoardImageFilterType::Pointer checkerBoard = CheckerBoardImageFilterType::New();

	//warnigns
	if ( in_filename.number < 2 ) {
		std::cout << "The number of input files has to be 2!" << std::endl;
		return EXIT_FAILURE;
	}

	if ( number_patterns < 2 ) {
		std::cout << "The number of patterns has to be at least 2! Setting number of patterns to 2." << std::endl;
		number_patterns = 2;
	}

	CheckerBoardImageFilterType::PatternArrayType patterns;

	for ( unsigned int i = 0; i < Dimension; i++ ) {
		patterns[i] = number_patterns;
	}

	reader1->SetFileName( ( ( VString* ) in_filename.vector )[0] );
	reader2->SetFileName( ( ( VString* ) in_filename.vector )[1] );
	reader1->Update();
	reader2->Update();
	checkerBoard->SetInput1( reader1->GetOutput() );
	checkerBoard->SetInput2( reader2->GetOutput() );
	checkerBoard->SetCheckerPattern( patterns );
	checkerBoard->Update();
	writer->SetFileName( out_filename );
	writer->SetInput( checkerBoard->GetOutput() );
	writer->Update();
	return 0;
};