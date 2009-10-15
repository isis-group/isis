/*
 * isisdotrans3d.cxx
 *
 *  Created on: October 15, 2009
 *      Author: tuerke
 */

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"

#include "itkTransformFileReader.h"
//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber

//command line parser options

VDictEntry TYPInterpolator[] = { {"Linear", 0}, {"BSpline", 1}, {"NearestNeighbor", 2}, {NULL}};

static VString in_filename = NULL;
static VString out_filename = NULL;
static VString trans_filename = NULL;
static VBoolean in_found, out_found, trans_found;
static VShort interpolator_type = 0;

static VOptionDescRec options[] = {
//requiered inputs
    {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the input image filename"}, {"out", VStringRepn, 1,
        &out_filename, &out_found, 0, "the output image filename"}, {"trans", VStringRepn, 1, &trans_filename,
        &trans_found, 0, "the transform filename"}, {"interpolator", VShortRepn, 1, &interpolator_type, VOptionalOpt,
        0, "The interpolator used to resample the image"}

};

int main(
    int argc, char* argv[]) {
	// DANGER! Kids don't try this at home! VParseCommand modifies the values of argc and argv!!!
	if(!VParseCommand(VNumber(options), options, &argc, argv) || !VIdentifyFiles(VNumber(options), options, "in",
	    &argc, argv, 0) || !VIdentifyFiles(VNumber (options), options, "out", &argc, argv, -1)) {
		VReportUsage(argv[0], VNumber(options), options, NULL);
		exit(1);
	}
	// VParseCommand reduces the argv vector to the name of the program and  unknown command line parameters.
	if(argc > 1) {
		VReportBadArgs(argc, argv);
		VReportUsage(argv[0], VNumber(options), options, NULL);
		exit(1);
	}

	//typedef section
	typedef unsigned short PixelType;
	const unsigned int Dimension = 3;

	return 0;
}
