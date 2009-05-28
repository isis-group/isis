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


namespace isis {
/** \class TimeStepExtractionFilter
 * \brief Filtertype for extracting a n-1 dimensional image
 *
 * \ingroup ImageFilters
 */

template< class TInputImage, class TOutputImage >
class ITK_EXPORT TimeStepExtractionFilter :
	public itk::ImageToImageFilter< TInputImage, TOutputImage >
{
public:
	itkStaticConstMacro( InputImageDimension, unsigned int,
			TInputImage::ImageDimension );
	itkStaticConstMacro( OutputImageDimension, unsigned int,
			TOutputImage::ImageDimension );

	typedef TInputImage								InputImageType;
	typedef TOutputImage							OutputImageType;

	/** Standard class typedefs. */
	typedef TimeStepExtractionFilter				Self;
	typedef itk::ImageToImageFilter
			< TInputImage, TOutputImage >			Superclass;
	typedef itk::SmartPointer< Self >				Pointer;
	typedef itk::SmartPointer< const Self >			ConstPointer;

	itkNewMacro( Self );

	/** Run-time type information (and related methods). */
	itkTypeMacro( TimeStepExtractionFilter, itk::ImageToImageFilter );

	typedef typename InputImageType::PixelType		InputPixelType;
	typedef typename OutputImageType::PixelType		OutputPixelType;

	typedef typename InputImageType::SizeType		InputSizeType;
	typedef typename InputImageType::IndexType		InputIndexType;

	typedef typename InputImageType::RegionType		InputRegionType;
	typedef typename OutputImageType::RegionType	OutputRegionType;

	itkSetMacro( RequestedTimeStep, unsigned int );


protected:
	TimeStepExtractionFilter();
	virtual ~TimeStepExtractionFilter() {}

	typedef itk::ExtractImageFilter< InputImageType, OutputImageType >
				ExtractFilterType;

	typename ExtractFilterType::Pointer				m_ExtractFilter;

	void GenerateData( void );

private:

	typename InputImageType::Pointer				m_InputImage;
	typename OutputImageType::Pointer				m_OutputImage;

	InputSizeType									m_InputSize;
	InputIndexType									m_InputIndex;


	InputRegionType 								m_InputRegion;
	InputRegionType 								m_DesiredRegion;

	unsigned int									m_RequestedTimeStep;
};

} // end namespace isis

#ifndef ITK_MANUAL_INSTANTIATION
#include "isisTimeStepExtractionFilter.txx"
#endif /* ITK_MANUAL_INSTANTIATION */

#endif /* __TIMESTEPEXTRACTIONFILTER_H_ */
