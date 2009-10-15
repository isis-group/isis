/*
 * isisdotrans3d.cxx
 *
 *  Created on: October 15, 2009
 *      Author: tuerke
 */

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkTransformFileReader.h"
//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber
//command line parser options

VDictEntry TYPInterpolator[] = { {"Linear", 0}, {"BSpline", 1}, {"NearestNeighbor", 2}, {NULL}};

static VString in_filename = NULL;
static VString out_filename = NULL;
static VString trans_filename = NULL;
static VString template_filename = NULL;
static VBoolean in_found, out_found, trans_found;
static VShort interpolator_type = 0;

static VOptionDescRec
        options[] = {
        //requiered inputs
            {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the input image filename"}, {"out", VStringRepn, 1,
                &out_filename, &out_found, 0, "the output image filename"}, {"trans", VStringRepn, 1, &trans_filename,
                &trans_found, 0, "the transform filename"},

            //non-required inputs
            {"interpolator", VShortRepn, 1, &interpolator_type, VOptionalOpt, 0,
                "The interpolator used to resample the image"}, {"tmp", VStringRepn, 1, &template_filename,
                VOptionalOpt, 0, "The template image"}

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
	typedef short PixelType;
	const unsigned int Dimension = 3;

	typedef itk::Image<PixelType, Dimension> InputImageType;
	typedef itk::Image<PixelType, Dimension> OutputImageType;

	typedef itk::ResampleImageFilter<InputImageType, OutputImageType> ResampleImageFilterType;
	typedef itk::CastImageFilter<InputImageType, OutputImageType> CastImageFilterType;

	typedef itk::ImageFileReader<InputImageType> ImageReaderType;
	typedef itk::ImageFileWriter<OutputImageType> ImageFileWriter;

	typedef const itk::Transform<double, Dimension, Dimension>* ConstTransformPointer;

	typedef itk::LinearInterpolateImageFunction<OutputImageType, double> LinearInterpolatorType;
	typedef itk::NearestNeighborInterpolateImageFunction<OutputImageType, double> NearestNeighborInterpolatorType;
	typedef itk::BSplineInterpolateImageFunction<OutputImageType, double> BSplineInterpolatorType;

	LinearInterpolatorType::Pointer linearInterpolator = LinearInterpolatorType::New();
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
	BSplineInterpolatorType::Pointer bsplineInterpolator = BSplineInterpolatorType::New();

	itk::TransformFileReader::Pointer transformFileReader = itk::TransformFileReader::New();
	ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();

	ImageReaderType::Pointer reader = ImageReaderType::New();
	ImageReaderType::Pointer templateReader = ImageReaderType::New();
	ImageFileWriter::Pointer writer = ImageFileWriter::New();

	writer->SetFileName(out_filename);

	//reading the input image
	reader->SetFileName(in_filename);
	reader->Update();

	//reading the transform from a file
	transformFileReader->SetFileName(trans_filename);
	transformFileReader->Update();

	itk::TransformFileReader::TransformListType *transformList = transformFileReader->GetTransformList();
	itk::TransformFileReader::TransformListType::const_iterator ti;
	ti = transformList->begin();

	//setting up the resample object
	resampler->SetTransform(static_cast<ConstTransformPointer> ((*ti).GetPointer()));
	resampler->SetInput(reader->GetOutput());

	if(!template_filename) {
		resampler->SetOutputDirection(reader->GetOutput()->GetDirection());
		resampler->SetOutputOrigin(reader->GetOutput()->GetOrigin());
		resampler->SetOutputSpacing(reader->GetOutput()->GetSpacing());
		resampler->SetSize(reader->GetOutput()->GetLargestPossibleRegion().GetSize());
	} else {
		templateReader->SetFileName(template_filename);
		templateReader->Update();
		resampler->SetOutputDirection(templateReader->GetOutput()->GetDirection());
		resampler->SetOutputOrigin(templateReader->GetOutput()->GetOrigin());
		resampler->SetOutputSpacing(templateReader->GetOutput()->GetSpacing());
		resampler->SetSize(templateReader->GetOutput()->GetLargestPossibleRegion().GetSize());
	}

	//setting up the interpolator
	switch(interpolator_type) {
	case 0:
		resampler->SetInterpolator(linearInterpolator);
		break;
	case 1:
		resampler->SetInterpolator(bsplineInterpolator);
		break;
	case 2:
		resampler->SetInterpolator(nearestNeighborInterpolator);
		break;
	}

	writer->SetInput(resampler->GetOutput());
	writer->Update();

	std::cout << "Done." << std::endl;

	return 0;
}
