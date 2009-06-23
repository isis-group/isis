/*
 * isisSTDEVMaskFilter.h
 *
 *  Created on: Jun 23, 2009
 *      Author: tuerke
 */

#ifndef ISISSTDEVMASKFILTER_H_
#define ISISSTDEVMASKFILTER_H_

#include "itkSmartPointer.h"
#include "itkImageToImageFilter.h"

namespace isis {

template< class TInputImage, class TOutputImage >
class STDEVMaskFilter
{
public:


	itkStaticConstMacro( InputImageDimension, unsigned int,
			TInputImage::ImageDimension);
	itkStaticConstMacro( OutputImageDimension, unsigned int,
			TOutputImage::ImageDimension);

	//Standard typedefs
	typedef STDEVMaskFilter							Self;
	typedef itk::SmartPointer< Self >				Pointer;
	typedef itk::SmartPointer< const Self >			ConstPointer;

	typedef TInputImage								InputImageType;
	typedef TOutputImage							OutputImageType;

	itkNewMacro( Self );

	typedef typename InputImageType::PixelType		InputPixelType;
	typedef typename OutputImageType::PixelType		OutputPixelType;

	typedef typename InputImageType::RegionType		InputRegionType;
	typedef typename OutputImageType::RegionType	OutputRegionType;

	typedef typename InputImageType::IndexType		InputIndexType;
	typedef typename OutputImageType::IndexType		OutputIndexType;

	typedef typename InputImageType::SpacingType	InputSpacingType;
	typedef typename OutputImageType::SpacingType	OutputSpacingType;

	typedef typename InputImageType::PointType		InputPointType;
	typedef typename OutputImageType::PointType		OutputPointType;

	typedef typename InputImageType::SizeType		InputSizeType;
	typedef typename OutputImageType::SizeType		OutputSizeType;

	typedef typename itk::NumericTraits< InputPixelType >
								::AccumulateType	SumType;
	typedef typename itk::NumericTraits< SumType >::RealType
													MeanType;



protected:
	STDEVMaskFilter();
	virtual ~STDEVMaskFilter() {}

private:
	STDEVMaskFilter( const Self& );

	void SetOutputParameters( void );

	typename InputImageType::ConstPointer			m_InputImage;
	typename OutputImageType::Pointer				m_OutputImage;

	InputIndexType									m_InputIndex;
	OutputIndexType									m_OutputIndex;

	InputSpacingType								m_InputSpacing;
	OutputSpacingType								m_OutputSpacing;

	InputSizeType									m_InputSize;
	OutputSizeType									m_OutputSize;

	InputRegionType									m_InputRegion;
	OutputRegionType								m_OutputRegion;

	InputPointType									m_InputOrigin;
	OutputPointType									m_OutputOrigin;

	unsigned int 									m_Timelength;






};

}


#include "isisSTDEVMaskFilter.txx"
#endif /* ISISSTDEVMASKFILTER_H_ */
