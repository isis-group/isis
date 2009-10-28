/*
 * isisdotrans3d.cxx
 *
 *  Created on: October 15, 2009
 *      Author: tuerke
 */

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageSeriesWriter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"

#include "extITK/isisTimeStepExtractionFilter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

#include "itkTileImageFilter.h"

#include "itkTransformFileReader.h"
//via command parser include
#include "viaio/option.h"
#include "viaio/mu.h" //this is required for VNumber
//command line parser options

VDictEntry TYPInterpolator[] = { {"Linear", 0}, {"BSpline", 1}, {"NearestNeighbor", 2}, {NULL}};

static VString in_filename = NULL;
static VString out_filename = NULL;
static VArgVector trans_filename;
static VString template_filename = NULL;
static VBoolean in_found, out_found, trans_found;
static VShort interpolator_type = 0;
static VArgVector resolution;
static VBoolean fmri;

static VOptionDescRec
        options[] = {
        //requiered inputs
            {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the input image filename"}, {"out", VStringRepn, 1,
                &out_filename, &out_found, 0, "the output image filename"}, {"trans", VStringRepn, 0, &trans_filename,
                &trans_found, 0, "the transform filename"},

            //non-required inputs
            {"interpolator", VShortRepn, 1, &interpolator_type, VOptionalOpt, 0,
                "The interpolator used to resample the image"}, {"tmp", VStringRepn, 1, &template_filename,
                VOptionalOpt, 0, "The template image"}, {"res", VFloatRepn, 0, (VPointer) &resolution, VOptionalOpt, 0,
                "The output resolution. One value for isotrop output"}, {"fmri", VBooleanRepn, 1, &fmri, VOptionalOpt,
                0, "Input and output image file are functional data"}

        };

#include "itkCommand.h"
class ProcessUpdate : public itk::Command
{
public:
	typedef ProcessUpdate Self;
	typedef itk::Command Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	itkNewMacro( Self )
	;
	void ShowProgressBar(
	    float progress) const {
		std::cout << progress * 100 << "%\r" << std::flush;
	}

protected:
	ProcessUpdate() {
	}
	;
public:
	typedef const itk::ProcessObject * ProcessPointer;

	void Execute(
	    itk::Object *caller, const itk::EventObject & event) {
		Execute((const itk::Object *) caller, event);
	}

	void Execute(
	    const itk::Object * object, const itk::EventObject & event) {
		ProcessPointer filter = dynamic_cast<ProcessPointer> (object);
		if(!(itk::ProgressEvent().CheckEvent(&event))) {
			return;
		}

		ShowProgressBar(filter->GetProgress());

	}

};

int main(

    int argc, char* argv[]) {

    // show revision information string constant
    std::cout << "Revision: " << SVN_REVISION << std::endl;

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
	const unsigned int fmriDimension = 4;

	typedef itk::Image<PixelType, Dimension> InputImageType;
	typedef itk::Image<PixelType, Dimension> OutputImageType;
	typedef itk::Image<PixelType, fmriDimension> FMRIInputType;
	typedef itk::Image<PixelType, fmriDimension> FMRIOutputType;

	typedef OutputImageType::SpacingType OutputSpacingType;
	typedef OutputImageType::SizeType OutputSizeType;

	typedef itk::ResampleImageFilter<InputImageType, OutputImageType> ResampleImageFilterType;
	typedef itk::CastImageFilter<InputImageType, OutputImageType> CastImageFilterType;

	typedef itk::ImageFileReader<InputImageType> ImageReaderType;
	typedef itk::ImageFileWriter<OutputImageType> ImageFileWriter;

	typedef itk::ImageFileReader<FMRIInputType> FMRIImageReaderType;
	typedef itk::ImageFileWriter<FMRIOutputType> FMRIImageWriterType;

	typedef itk::ImageSeriesWriter<FMRIOutputType, OutputImageType> ImageSeriesWriterType;

	typedef isis::extitk::TimeStepExtractionFilter<FMRIInputType, InputImageType> TimeStepExtractionFilterType;

	typedef itk::TileImageFilter<OutputImageType, FMRIOutputType> TileImageFitlerType;

	typedef const itk::Transform<double, Dimension, Dimension>* ConstTransformPointer;

	typedef itk::LinearInterpolateImageFunction<OutputImageType, double> LinearInterpolatorType;
	typedef itk::NearestNeighborInterpolateImageFunction<OutputImageType, double> NearestNeighborInterpolatorType;
	typedef itk::BSplineInterpolateImageFunction<OutputImageType, double> BSplineInterpolatorType;

	LinearInterpolatorType::Pointer linearInterpolator = LinearInterpolatorType::New();
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
	BSplineInterpolatorType::Pointer bsplineInterpolator = BSplineInterpolatorType::New();

	itk::TransformFileReader::Pointer transformFileReader = itk::TransformFileReader::New();
	ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
	CastImageFilterType::Pointer caster = CastImageFilterType::New();

	ProcessUpdate::Pointer progressObserver = ProcessUpdate::New();

	TimeStepExtractionFilterType::Pointer timeStepExtractionFilter = TimeStepExtractionFilterType::New();

	ImageReaderType::Pointer reader = ImageReaderType::New();
	ImageReaderType::Pointer templateReader = ImageReaderType::New();
	ImageFileWriter::Pointer writer = ImageFileWriter::New();
	FMRIImageReaderType::Pointer fmriReader = FMRIImageReaderType::New();
	FMRIImageWriterType::Pointer fmriWriter = FMRIImageWriterType::New();
	ImageSeriesWriterType::Pointer seriesWriter = ImageSeriesWriterType::New();

	TileImageFitlerType::Pointer tileImageFilter = TileImageFitlerType::New();

	InputImageType::Pointer inputImage = InputImageType::New();

	OutputSpacingType outputSpacing;
	OutputSizeType outputSize;

	if(!trans_filename.number) {
		std::cout << "No transform specified!!" << std::endl;
		return EXIT_FAILURE;
	}

	//reading the input image
	if(!fmri) {
		resampler->AddObserver(itk::ProgressEvent(), progressObserver);
		reader->SetFileName(in_filename);
		reader->Update();
		writer->SetFileName(out_filename);
	}
	if(fmri) {
		fmriReader->SetFileName(in_filename);
		fmriReader->Update();
		timeStepExtractionFilter->SetInput(fmriReader->GetOutput());
		fmriWriter->SetFileName(out_filename);
	}

	//if template file is specified by the user
	if(template_filename) {
		templateReader->SetFileName(template_filename);
		templateReader->Update();
	}

	//setting up the output resolution
	if(resolution.number) {

		if(static_cast<unsigned int> (resolution.number) < Dimension) {
			//user has specified less than 3 resolution values->sets isotrop resolution with the first typed value
			outputSpacing.Fill(((VFloat *) resolution.vector)[0]);

		}
		if(resolution.number >= 3) {
			//user has specified at least 3 values -> sets anisotrop resolution
			for(unsigned int i = 0; i < 3; i++) {
				outputSpacing[i] = ((VFloat *) resolution.vector)[i];
			}
		}
	}

	if(!resolution.number) {
		if(template_filename) {
			outputSpacing = templateReader->GetOutput()->GetSpacing();
			outputSize = templateReader->GetOutput()->GetLargestPossibleRegion().GetSize();
		}
		if(!template_filename) {
			outputSpacing = reader->GetOutput()->GetSpacing();
			outputSize = reader->GetOutput()->GetLargestPossibleRegion().GetSize();
		}
	}

	if(resolution.number and template_filename) {
		for(unsigned int i = 0; i < 3; i++) {
			//output spacing = (template size / output resolution) * template resolution
			outputSize[i] = ((templateReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i]) / outputSpacing[i])
			        * templateReader->GetOutput()->GetSpacing()[i];
		}
	}
	if(resolution.number and !template_filename) {
		for(unsigned int i = 0; i < 3; i++) {
			//output spacing = (moving size / output resolution) * moving resolution
			outputSize[i] = ((reader->GetOutput()->GetLargestPossibleRegion().GetSize()[i]) / outputSpacing[i])
			        * reader->GetOutput()->GetSpacing()[i];
		}
	}
	//reading the transform from a file

	unsigned int number_trans = trans_filename.number;
	if(number_trans > 1) {
		//merging the transforms to one final transform
		//TODO
	}
	transformFileReader->SetFileName(((VStringConst *) trans_filename.vector)[0]);
	transformFileReader->Update();

	itk::TransformFileReader::TransformListType *transformList = transformFileReader->GetTransformList();
	itk::TransformFileReader::TransformListType::const_iterator ti;
	ti = transformList->begin();

	//setting up the resample object
	resampler->SetTransform(static_cast<ConstTransformPointer> ((*ti).GetPointer()));

	if(!template_filename) {
		resampler->SetOutputDirection(reader->GetOutput()->GetDirection());
		resampler->SetOutputOrigin(reader->GetOutput()->GetOrigin());
	} else {
		resampler->SetOutputDirection(templateReader->GetOutput()->GetDirection());
		resampler->SetOutputOrigin(templateReader->GetOutput()->GetOrigin());
	}
	resampler->SetOutputSpacing(outputSpacing);
	resampler->SetSize(outputSize);

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

	if(!fmri) {
		resampler->SetInput(reader->GetOutput());
		writer->SetInput(resampler->GetOutput());
		writer->Update();
	}

	if(fmri) {
		OutputImageType::SizeType fmriOutputSize;
		OutputImageType::SpacingType fmriOutputSpacing;
		OutputImageType::DirectionType fmriOutputDirection;
		OutputImageType::PointType fmriOutputOrigin;
		for(unsigned int i = 0; i < 3; i++) {
			fmriOutputOrigin[i] = fmriReader->GetOutput()->GetOrigin()[i];
			fmriOutputSpacing[i] = fmriReader->GetOutput()->GetSpacing()[i];
			fmriOutputSize[i] = fmriReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i];
			fmriOutputOrigin[i] = fmriReader->GetOutput()->GetOrigin()[i];
			for(unsigned int j = 0; j < 3; j++) {
				fmriOutputDirection[j][i] = fmriReader->GetOutput()->GetDirection()[j][i];
			}
		}
		resampler->SetOutputDirection(fmriOutputDirection);
		resampler->SetOutputSpacing(fmriOutputSpacing);
		resampler->SetSize(fmriOutputSize);
		resampler->SetOutputOrigin(fmriOutputOrigin);

		itk::FixedArray<unsigned int, 4> layout;
		layout[0] = 1;
		layout[1] = 1;
		layout[2] = 1;
		layout[3] = 0;
		InputImageType::IndexType pixel;
		pixel.Fill(8);
		const unsigned int numberOfTimeSteps = fmriReader->GetOutput()->GetLargestPossibleRegion().GetSize()[3];
		for(unsigned int timestep = 0; timestep < numberOfTimeSteps; timestep++) {
			std::cout << "Resampling timestep: " << timestep << "...\r" << std::flush;
			timeStepExtractionFilter->SetRequestedTimeStep(timestep);
			timeStepExtractionFilter->Update();
			resampler->SetInput(timeStepExtractionFilter->GetOutput());
			resampler->Update();
			tileImageFilter->PushBackInput(resampler->GetOutput());
		}
		tileImageFilter->SetLayout(layout);
		tileImageFilter->Update();
		fmriWriter->SetInput(tileImageFilter->GetOutput());
		fmriWriter->Update();

	}
	std::cout << "\nDone.    " << std::endl;

	return 0;
}
