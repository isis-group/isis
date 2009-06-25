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
#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkBinaryThresholdImageFilter.h"


#include <vector>
#include <algorithm>
#include <cmath>


namespace isis {

template< class TInputImage, class TOutputImage >
class ITK_EXPORT STDEVMaskFilter :
public itk::ImageToImageFilter< TInputImage, TOutputImage >
{
public:

	STDEVMaskFilter();
	itkStaticConstMacro( InputImageDimension, unsigned int,
			TInputImage::ImageDimension);
	itkStaticConstMacro( OutputImageDimension, unsigned int,
			TOutputImage::ImageDimension);

	//Standard typedefs
	typedef STDEVMaskFilter							Self;
	typedef itk::SmartPointer< Self >				Pointer;
	typedef itk::SmartPointer< const Self >			ConstPointer;
	typedef itk::ImageToImageFilter< TInputImage, TOutputImage >
													itkSuperclass;



	typedef typename itkSuperclass::InputImagePointer
												InputImagePointer;

	typedef TInputImage								InputImageType;
	typedef TOutputImage							OutputImageType;

	typedef typename InputImageType::ConstPointer	InputImageConstPointer;
	typedef typename OutputImageType::Pointer		OutputImagePointer;

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
	itkNewMacro( Self );

	itkTypeMacro( STDEVMaskFilter, itk::ImageToImageFilter );

	itkSetMacro( Begin, unsigned int );



	typedef typename itk::NumericTraits< OutputPixelType >
								::AccumulateType	SumType;
	typedef typename itk::NumericTraits< SumType >::RealType
													MeanType;

	typedef typename itk::ImageLinearConstIteratorWithIndex
					 < InputImageType >				IteratorType;

	virtual void Update( void );


	OutputImagePointer GetOutput( void );


	virtual ~STDEVMaskFilter() {}

private:

	STDEVMaskFilter( const Self& );

	void SetOutputParameters( void );

	InputImageConstPointer							m_InputImage;
	OutputImagePointer								m_OutputImage;

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
	unsigned int 									m_Begin;






};

}


#include "isisSTDEVMaskFilter.txx"
#endif /* ISISSTDEVMASKFILTER_H_ */
