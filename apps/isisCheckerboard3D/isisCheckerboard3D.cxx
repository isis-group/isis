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
static VString out_filename=0;
static VShort number_patterns=5;
static VBoolean in_found, out_found;

static VOptionDescRec
options[] = {
    //requiered inputs
    {"in", VStringRepn, 0, (VPointer) &in_filename, &in_found, 0, "the input image filenames"}, {"out", VStringRepn, 1,
            &out_filename, &out_found, 0, "the output image filename"}
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

	
	
	
	
	
	
	
	
	const unsigned int Dimension = 3;
	typedef short PixelType;
	
	
	
	return 0;
};