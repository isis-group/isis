/*
 * timeStepExtractorFilter.h
 *
 *  Created on: May 15, 2009
 *      Author: tuerke
 */


#ifndef _TIMESTEPEXTRACTORFILTER_H_
#define _TIMESTEPEXTRACTORFILTER_H_

#include "itkImage.h"
#include "itkExtractImageFilter.h"


template< class TImageInput, class TImageOutput >
class TimeStepExtractorFilter
{


public:
	TimeStepExtractorFilter();
	//virtual ~timeStepExtractorFilter();
	typedef TimeStepExtractorFilter Self;

	typedef TImageInput								InputImageType;
	typedef TImageOutput							OutputImageType;
	typedef typename InputImageType::Pointer 		InputImagePointer;
	typedef typename OutputImageType::Pointer		OutputImagePointer;
	typedef typename InputImageType::RegionType 	ImageRegionType;
	typedef typename InputImageType::SizeType		ImageSizeType;
	typedef typename InputImageType::IndexType		ImageIndexType;
	typedef typename InputImageType::PixelType		ImagePixelType;

	typedef typename itk::ExtractImageFilter< InputImageType, OutputImageType >
			ExtractFilterType;


	void Start( void );

	//setter methods
	void SetSliceNumber( unsigned int );
	void SetInputImage( InputImagePointer );



	//getter methods
	unsigned int GetSliceNumber( void );
	OutputImagePointer GetOutputImage( void );
	unsigned int GetNumberOfTimeSteps( void );



private:
	unsigned int m_sliceNumber;
	unsigned int m_InputDimension;
	unsigned int m_OutputDimension;
	unsigned int m_NumberOfTimeSteps;


	InputImagePointer m_InputImage;
	OutputImagePointer m_OutputImage;

	ImageSizeType m_InputImageSize;



	ImageRegionType m_InputRegion;
	ImageRegionType m_OutputRegion;

};

#include "timeStepExtractorFilter.txx"
#endif /* _TIMESTEPEXTRACTORFILTER_H_ */
