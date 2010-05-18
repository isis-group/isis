/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 *****************************************************************/


#ifndef ISISSTDEVMASKFILTER_H_
#define ISISSTDEVMASKFILTER_H_

#include "itkSmartPointer.h"
#include "itkImageToImageFilter.h"
#include "itkImageLinearConstIteratorWithIndex.h"
#include "itkBinaryThresholdImageFilter.h"

#include <vector>
#include <algorithm>
#include <cmath>

namespace isis
{
namespace extitk
{

template<class TInputImage, class TOutputImage>
class ITK_EXPORT STDEVMaskFilter: public itk::ImageToImageFilter < TInputImage,
		TOutputImage >
{
public:

	STDEVMaskFilter();
	itkStaticConstMacro( InputImageDimension, unsigned int,
						 TInputImage::ImageDimension );
	itkStaticConstMacro( OutputImageDimension, unsigned int,
						 TOutputImage::ImageDimension );

	//Standard typedefs
	typedef STDEVMaskFilter Self;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;
	typedef itk::ImageToImageFilter<TInputImage, TOutputImage> itkSuperclass;

	typedef typename itkSuperclass::InputImagePointer InputImagePointer;

	typedef TInputImage InputImageType;
	typedef TOutputImage OutputImageType;

	typedef typename InputImageType::ConstPointer InputImageConstPointer;
	typedef typename OutputImageType::Pointer OutputImagePointer;

	typedef typename InputImageType::PixelType InputPixelType;
	typedef typename OutputImageType::PixelType OutputPixelType;

	typedef typename InputImageType::RegionType InputRegionType;
	typedef typename OutputImageType::RegionType OutputRegionType;

	typedef typename InputImageType::IndexType InputIndexType;
	typedef typename OutputImageType::IndexType OutputIndexType;

	typedef typename InputImageType::SpacingType InputSpacingType;
	typedef typename OutputImageType::SpacingType OutputSpacingType;

	typedef typename InputImageType::PointType InputPointType;
	typedef typename OutputImageType::PointType OutputPointType;

	typedef typename InputImageType::SizeType InputSizeType;
	typedef typename OutputImageType::SizeType OutputSizeType;
	itkNewMacro( Self )
	;

	itkTypeMacro( STDEVMaskFilter, itk::ImageToImageFilter )
	;

	itkSetMacro( Begin, unsigned int )
	;

	typedef typename itk::NumericTraits<OutputPixelType>::AccumulateType
	SumType;
	typedef typename itk::NumericTraits<SumType>::RealType MeanType;

	typedef typename itk::ImageLinearConstIteratorWithIndex<InputImageType>
	IteratorType;

	virtual void Update( void );

	OutputImagePointer GetOutput( void );

	virtual ~STDEVMaskFilter() {
	}

private:

	STDEVMaskFilter( const Self& );

	void SetOutputParameters( void );

	InputImageConstPointer m_InputImage;
	OutputImagePointer m_OutputImage;

	InputIndexType m_InputIndex;
	OutputIndexType m_OutputIndex;

	InputSpacingType m_InputSpacing;
	OutputSpacingType m_OutputSpacing;

	InputSizeType m_InputSize;
	OutputSizeType m_OutputSize;

	InputRegionType m_InputRegion;
	OutputRegionType m_OutputRegion;

	InputPointType m_InputOrigin;
	OutputPointType m_OutputOrigin;

	unsigned int m_Timelength;
	unsigned int m_Begin;

};

}
}

#include "isisSTDEVMaskFilter.txx"
#endif /* ISISSTDEVMASKFILTER_H_ */
