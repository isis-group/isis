/*
 * isisTimeStepExtractionFilter.h
 *
 *  Created on: May 15, 2009
 *      Author: tuerke
 */

#ifndef __TIMESTEPEXTRACTIONFILTER_H_
#define __TIMESTEPEXTRACTIONFILTER_H_

#include "itkImageToImageFilter.h"
#include "itkExtractImageFilter.h"
#include "itkSmartPointer.h"

namespace isis
{
namespace extitk
{
/** \class TimeStepExtractionFilter
 * \brief Filtertype for extracting a n-1 dimensional image
 *
 * \ingroup ImageFilters
 */

template<class TInputImage, class TOutputImage>
class ITK_EXPORT TimeStepExtractionFilter: public itk::ImageToImageFilter <
	TInputImage, TOutputImage >
{
public:
	itkStaticConstMacro( InputImageDimension, unsigned int,
						 TInputImage::ImageDimension );
	itkStaticConstMacro( OutputImageDimension, unsigned int,
						 TOutputImage::ImageDimension );

	typedef TInputImage InputImageType;
	typedef TOutputImage OutputImageType;

	/** Standard class typedefs. */
	typedef TimeStepExtractionFilter Self;
	typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;

	typedef typename Superclass::InputImagePointer InputImagePointer;

	itkNewMacro( Self )
	;

	/** Run-time type information (and related methods). */
	itkTypeMacro( TimeStepExtractionFilter, itk::ImageToImageFilter )
	;

	typedef typename InputImageType::PixelType InputPixelType;
	typedef typename OutputImageType::PixelType OutputPixelType;

	typedef typename InputImageType::SizeType InputSizeType;
	typedef typename InputImageType::IndexType InputIndexType;

	typedef typename TInputImage::RegionType InputRegionType;
	typedef typename TOutputImage::RegionType OutputRegionType;

	itkSetMacro( RequestedTimeStep, unsigned int )
	;
	itkSetMacro( RequestedTimeRangeBegin, unsigned int )
	;
	itkSetMacro( RequestedTimeRangeEnd, unsigned int )
	;
	void Update( void );
	typename OutputImageType::Pointer GetOutput() const;
protected:
	TimeStepExtractionFilter();
	virtual ~TimeStepExtractionFilter() {
	}

	typedef itk::ExtractImageFilter<InputImageType, OutputImageType>
	ExtractFilterType;

	typename ExtractFilterType::Pointer m_ExtractFilter;



private:
	TimeStepExtractionFilter( const Self & );

	typename InputImageType::Pointer m_InputImage;
	typename OutputImageType::Pointer m_OutputImage;

	InputSizeType m_InputSize;
	InputIndexType m_InputIndex;

	InputRegionType m_InputImageRegion;
	InputRegionType m_DesiredRegion;
	InputRegionType m_ExtractionRegion;
	OutputRegionType m_OutputImageRegion;

	unsigned int m_RequestedTimeStep;
	unsigned int m_RequestedTimeRangeBegin;
	unsigned int m_RequestedTimeRangeEnd;

};

} // end namesapce extitk
} // end namespace isis

#if ITK_TEMPLATE_TXX
#include "isisTimeStepExtractionFilter.txx"
#endif /* ITK_TEMPLATE_TXX */

#endif /* __TIMESTEPEXTRACTIONFILTER_H_ */
