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
#include "itkLinearInterpolateImageFunction.h"
#include "itkWindowedSincInterpolateImageFunction.h"

#include "extITK/isisTimeStepExtractionFilter.h"
#include "extITK/isisTransformMerger2D.hpp"
#include "extITK/isisIterationObserver.h"

#include "itkResampleImageFilter.h"
#include "itkWarpImageFilter.h"
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
static VString vtrans_filename;
static VString template_filename = NULL;
static VBoolean in_found, out_found, trans_found;
static VShort interpolator_type = 0;
static VArgVector resolution;
static VBoolean fmri;
static VBoolean use_inverse = false;
static VShort number_threads = 1;

static VOptionDescRec options[] = {
//requiered inputs
    {"in", VStringRepn, 1, &in_filename, &in_found, 0, "the input image filename"}, {"out", VStringRepn, 1,
        &out_filename, &out_found, 0, "the output image filename"},

    //non-required inputs
    {"trans", VStringRepn, 0, &trans_filename, &trans_found, 0, "the transform filename"}, {"interpolator", VShortRepn,
        1, &interpolator_type, VOptionalOpt, 0, "The interpolator used to resample the image"}, {"ref", VStringRepn, 1,
        &template_filename, VOptionalOpt, 0, "The template image"}, {"reso", VFloatRepn, 0, (VPointer) &resolution,
        VOptionalOpt, 0, "The output resolution. One value for isotrop output"}, {"fmri", VBooleanRepn, 1, &fmri,
        VOptionalOpt, 0, "Input and output image file are functional data"}, {"vtrans", VStringRepn, 1,
        &vtrans_filename, VOptionalOpt, 0, "Vector deformation field"}, {"use_inverse", VBooleanRepn, 1, &use_inverse,
	VOptionalOpt, 0, "Using the inverse of the transform"}, {"j" , VShortRepn, 1, &number_threads, VOptionalOpt, 0 ,"Number of threads"}

};

#include "itkCommand.h"
#include <boost/concept_check.hpp>


int main(

    int argc, char* argv[]) {
	// show revision information string constant
	std::cout << "Revision: " << _SVN_REVISION << std::endl << std::flush;

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
	const unsigned int Dimension = 2;
	const unsigned int fmriDimension = 3;

	typedef itk::Vector<float, Dimension> VectorType;
	typedef itk::Image<VectorType, Dimension> DeformationFieldType;
	typedef itk::Image<PixelType, Dimension> InputImageType;
	typedef itk::Image<PixelType, Dimension> OutputImageType;
	typedef itk::Image<PixelType, fmriDimension> FMRIInputType;
	typedef itk::Image<PixelType, fmriDimension> FMRIOutputType;

	typedef itk::ResampleImageFilter<InputImageType, OutputImageType> ResampleImageFilterType;
	typedef itk::WarpImageFilter<OutputImageType, OutputImageType, DeformationFieldType> WarpImageFilterType;
	typedef itk::CastImageFilter<InputImageType, OutputImageType> CastImageFilterType;

	typedef itk::ImageFileReader<InputImageType> ImageReaderType;
	typedef itk::ImageFileWriter<OutputImageType> ImageFileWriter;

	typedef itk::ImageFileReader<FMRIInputType> FMRIImageReaderType;
	typedef itk::ImageFileWriter<FMRIOutputType> FMRIImageWriterType;
	typedef itk::ImageFileReader<DeformationFieldType> DeformationFieldReaderType;

	typedef itk::ImageSeriesWriter<FMRIOutputType, OutputImageType> ImageSeriesWriterType;

	typedef isis::extitk::TimeStepExtractionFilter<FMRIInputType, InputImageType> TimeStepExtractionFilterType;

	typedef itk::TileImageFilter<OutputImageType, FMRIOutputType> TileImageFitlerType;

	typedef const itk::Transform<double, Dimension, Dimension>* ConstTransformPointer;
	typedef itk::Transform<double, Dimension, Dimension>* TransformPointer;
	itk::Transform<double, Dimension, Dimension>::ParametersType parameters;

	typedef itk::LinearInterpolateImageFunction<OutputImageType, double> LinearInterpolatorType;
	typedef itk::NearestNeighborInterpolateImageFunction<OutputImageType, double> NearestNeighborInterpolatorType;
	typedef itk::BSplineInterpolateImageFunction<OutputImageType, double> BSplineInterpolatorType;

	LinearInterpolatorType::Pointer linearInterpolator = LinearInterpolatorType::New();
	NearestNeighborInterpolatorType::Pointer nearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
	BSplineInterpolatorType::Pointer bsplineInterpolator = BSplineInterpolatorType::New();

	itk::TransformFileReader::Pointer transformFileReader = itk::TransformFileReader::New();
	ResampleImageFilterType::Pointer resampler = ResampleImageFilterType::New();
	WarpImageFilterType::Pointer warper = WarpImageFilterType::New();
	CastImageFilterType::Pointer caster = CastImageFilterType::New();

	isis::extitk::ProcessUpdate::Pointer progressObserver = isis::extitk::ProcessUpdate::New();

	TimeStepExtractionFilterType::Pointer timeStepExtractionFilter = TimeStepExtractionFilterType::New();
	isis::extitk::TransformMerger2D* transformMerger = new isis::extitk::TransformMerger2D;

	DeformationFieldReaderType::Pointer deformationFieldReader = DeformationFieldReaderType::New();
	ImageReaderType::Pointer reader = ImageReaderType::New();
	ImageReaderType::Pointer templateReader = ImageReaderType::New();
	ImageFileWriter::Pointer writer = ImageFileWriter::New();
	ImageFileWriter::Pointer testwriter = ImageFileWriter::New();
	FMRIImageReaderType::Pointer fmriReader = FMRIImageReaderType::New();
	FMRIImageWriterType::Pointer fmriWriter = FMRIImageWriterType::New();
	ImageSeriesWriterType::Pointer seriesWriter = ImageSeriesWriterType::New();

	TileImageFitlerType::Pointer tileImageFilter = TileImageFitlerType::New();

	OutputImageType::SpacingType outputSpacing;
	OutputImageType::SizeType outputSize;
	OutputImageType::PointType outputOrigin;
	OutputImageType::DirectionType outputDirection;

	OutputImageType::SizeType fmriOutputSize;
	OutputImageType::SpacingType fmriOutputSpacing;
	OutputImageType::DirectionType fmriOutputDirection;
	OutputImageType::PointType fmriOutputOrigin;
	InputImageType::Pointer tmpImage = InputImageType::New();
	//transform object used for inverse transform
	itk::MatrixOffsetTransformBase<double, Dimension, Dimension>::Pointer transform = itk::MatrixOffsetTransformBase<double, Dimension, Dimension>::New();
 			
	if(!trans_filename.number and !vtrans_filename) {
		std::cout << "No transform specified!!" << std::endl;
		return EXIT_FAILURE;
	}
	resampler->SetNumberOfThreads(number_threads);
	warper->SetNumberOfThreads(number_threads);
	progress_timer time;
	if(!fmri) {
	    reader->SetFileName(in_filename);
	    reader->Update();
	}
	if(fmri) {
	    fmriReader->SetFileName(in_filename);
	    fmriReader->Update();
	}
	//if template file is specified by the user
	if(template_filename) {
		templateReader->SetFileName(template_filename);
		templateReader->Update();
		outputDirection = templateReader->GetOutput()->GetDirection();
		outputOrigin = templateReader->GetOutput()->GetOrigin();
	}
	if(!template_filename) {
		outputDirection = reader->GetOutput()->GetDirection();
		outputOrigin = reader->GetOutput()->GetOrigin();
		
	    }
	if(trans_filename.number) {
		unsigned int number_trans = trans_filename.number;
		if(number_trans > 1) {

			for(unsigned int i = 0; i < number_trans; i++) {
				itk::TransformFileReader::TransformListType* transformList =
				        new itk::TransformFileReader::TransformListType;
				transformFileReader->SetFileName(((VStringConst *) trans_filename.vector)[i]);
				transformFileReader->Update();
				transformList = transformFileReader->GetTransformList();
				itk::TransformFileReader::TransformListType::const_iterator ti = transformList->begin();
				transformMerger->push_back((*ti).GetPointer());
			}
			transformMerger->setTemplateImage(templateReader->GetOutput());
			transformMerger->merge();
			warper->SetDeformationField(transformMerger->getTransform());
		}
		if(number_trans == 1) {
			transformFileReader->SetFileName(((VStringConst *) trans_filename.vector)[0]);
			transformFileReader->Update();

			itk::TransformFileReader::TransformListType *transformList = transformFileReader->GetTransformList();
			itk::TransformFileReader::TransformListType::const_iterator ti;
			ti = transformList->begin();
			//setting up the resample object
			if(use_inverse) {
// 			    transform->SetParameters(static_cast<TransformPointer>((*ti).GetPointer())->GetInverseTransform()->GetParameters());
			    resampler->SetTransform(transform);
	
			}
			if(!use_inverse) {
			    resampler->SetTransform(static_cast<ConstTransformPointer> ((*ti).GetPointer()));
			 
			}
		}

	}

	if(vtrans_filename) {
		deformationFieldReader->SetFileName(vtrans_filename);
		deformationFieldReader->Update();
	}

	//setting up the output resolution
	if(resolution.number) {

		if(static_cast<unsigned int> (resolution.number) < Dimension) {
			//user has specified less than 3 resolution values->sets isotrop resolution with the first typed value
			outputSpacing.Fill(((VFloat *) resolution.vector)[0]);

		}
		if(static_cast<unsigned int>(resolution.number) >= Dimension) {
			//user has specified at least 3 values -> sets anisotrop resolution
			for(unsigned int i = 0; i < Dimension; i++) {
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
		for(unsigned int i = 0; i < Dimension; i++) {
			//output spacing = (template size / output resolution) * template resolution
			outputSize[i] = ((templateReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i]) / outputSpacing[i])
			        * templateReader->GetOutput()->GetSpacing()[i];
		}
	}
	
	if(resolution.number and !template_filename) {
		for(unsigned int i = 0; i < Dimension; i++) {
			//output spacing = (moving size / output resolution) * moving resolution
			if(!fmri) {
				outputSize[i] = ((reader->GetOutput()->GetLargestPossibleRegion().GetSize()[i]) / outputSpacing[i])
				        * reader->GetOutput()->GetSpacing()[i];
			}
			if(fmri) {
				outputSize[i] = ((fmriReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i]) / outputSpacing[i])
				        * fmriReader->GetOutput()->GetSpacing()[i];
			}

		}
	}
	//setting up the interpolator
	switch(interpolator_type) {
	case 0:
		resampler->SetInterpolator(linearInterpolator);
		warper->SetInterpolator(linearInterpolator);
		break;
	case 1:
		resampler->SetInterpolator(bsplineInterpolator);
		warper->SetInterpolator(bsplineInterpolator);
		break;
	case 2:
		resampler->SetInterpolator(nearestNeighborInterpolator);
		warper->SetInterpolator(nearestNeighborInterpolator);
		break;
	}

	if(!fmri) {

		writer->SetFileName(out_filename);
		if(!vtrans_filename and trans_filename.number == 1) {
			resampler->AddObserver(itk::ProgressEvent(), progressObserver);
			tmpImage = reader->GetOutput();
			resampler->SetInput(reader->GetOutput());
			resampler->SetOutputSpacing(outputSpacing);
			resampler->SetSize(outputSize);
			resampler->SetOutputOrigin(outputOrigin);
			resampler->SetOutputDirection(outputDirection);
			writer->SetInput(resampler->GetOutput());
			writer->Update();
		}
		if(vtrans_filename or trans_filename.number > 1) {
			warper->AddObserver(itk::ProgressEvent(), progressObserver);
			warper->SetOutputDirection(outputDirection);
			warper->SetOutputOrigin(outputOrigin);
			warper->SetOutputSize(outputSize);
			warper->SetOutputSpacing(outputSpacing);
			warper->SetInput(reader->GetOutput());
			if(trans_filename.number == 0) {
				warper->SetDeformationField(deformationFieldReader->GetOutput());
			}
			writer->SetInput(warper->GetOutput());
			writer->Update();
		}

	}

	if(fmri) {
		
		timeStepExtractionFilter->SetInput(fmriReader->GetOutput());
		fmriWriter->SetFileName(out_filename);
		if(template_filename) {
 			fmriOutputOrigin = templateReader->GetOutput()->GetOrigin();
 			fmriOutputDirection = templateReader->GetOutput()->GetDirection();			
		}
		for(unsigned int i = 0; i < Dimension; i++) {
			if(resolution.number) {
				fmriOutputSpacing[i] = outputSpacing[i];
				fmriOutputSize[i] = outputSize[i];
			} else {
				if(!template_filename) {
				    fmriOutputSpacing[i] = fmriReader->GetOutput()->GetSpacing()[i];
				    fmriOutputSize[i] = fmriReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i];
				}
				if(template_filename) {
				    fmriOutputSpacing[i] = templateReader->GetOutput()->GetSpacing()[i];
				    fmriOutputSize[i] = templateReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i];
				}
			}
			if(!template_filename) {
				fmriOutputOrigin[i] = fmriReader->GetOutput()->GetOrigin()[i];
				for(unsigned int j = 0; j < Dimension; j++) {
					fmriOutputDirection[j][i] = fmriReader->GetOutput()->GetDirection()[j][i];
				}
			}
		}
		if(trans_filename.number) {
			resampler->SetOutputDirection(fmriOutputDirection);
			resampler->SetOutputSpacing(fmriOutputSpacing);
			resampler->SetSize(fmriOutputSize);
			resampler->SetOutputOrigin(fmriOutputOrigin);
		}
		if(vtrans_filename) {
			warper->SetOutputDirection(fmriOutputDirection);
			warper->SetOutputOrigin(fmriOutputOrigin);
			warper->SetOutputSize(fmriOutputSize);
			warper->SetOutputSpacing(fmriOutputSpacing);
			warper->SetInput(reader->GetOutput());
			if(trans_filename.number == 0) {
				warper->SetDeformationField(deformationFieldReader->GetOutput());
			}
		}

		itk::FixedArray<unsigned int, fmriDimension> layout;
		layout[0] = 1;
		layout[1] = 1;
		layout[2] = 0;
		const unsigned int numberOfTimeSteps = fmriReader->GetOutput()->GetLargestPossibleRegion().GetSize()[2];
		OutputImageType::Pointer tileImage;
		std::cout << std::endl;
		reader->SetFileName(in_filename);
		reader->Update();
		for(unsigned int timestep = 0; timestep < numberOfTimeSteps; timestep++) {
			std::cout << "Resampling timestep: " << timestep << "...\r" << std::flush;
			timeStepExtractionFilter->SetRequestedTimeStep(timestep);
			timeStepExtractionFilter->Update();
			if(trans_filename.number) {
				tmpImage = timeStepExtractionFilter->GetOutput();
// 					std::cout << tmpImage << std::endl;
				tmpImage->SetDirection(reader->GetOutput()->GetDirection());
				tmpImage->SetOrigin(reader->GetOutput()->GetOrigin());

				resampler->SetInput(tmpImage);
				resampler->Update();
				tileImage = resampler->GetOutput();
			}
			if(vtrans_filename) {
				warper->SetInput(timeStepExtractionFilter->GetOutput());
				warper->Update();
				tileImage = warper->GetOutput();
			}
			tileImage->Update();
			tileImage->DisconnectPipeline();
			tileImageFilter->PushBackInput(tileImage);
		}
		tileImageFilter->SetLayout(layout);
		tileImageFilter->Update();
		fmriWriter->SetInput(tileImageFilter->GetOutput());
		fmriWriter->Update();

	}
	std::cout << std::endl << "Done.    " << std::endl;

	return 0;
}
