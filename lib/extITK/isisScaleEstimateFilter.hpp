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

#ifndef ISISSCALEESTIMATEFILTER_HPP_
#define ISISSCALEESTIMATEFILTER_HPP_


#include <itkOtsuThresholdImageCalculator.h>
#include <itkRecursiveGaussianImageFilter.h>
#include <itkImageMomentsCalculator.h>

#include <boost/foreach.hpp>

namespace isis {

namespace extitk{


template<class InputImageType1, class InputImageType2>
class ScaleEstimateFilter
{

	typedef itk::OtsuThresholdImageCalculator<InputImageType1> OtsuCalculatorType1;
	typedef itk::OtsuThresholdImageCalculator<InputImageType2> OtsuCalculatorType2;

	typedef itk::RecursiveGaussianImageFilter<InputImageType1, InputImageType1> GaussianFilterType1;
	typedef itk::RecursiveGaussianImageFilter<InputImageType2, InputImageType2> GaussianFilterType2;


	typedef itk::ImageMomentsCalculator<InputImageType1> MomentsCalculatorType1;
	typedef itk::ImageMomentsCalculator<InputImageType2> MomentsCalculatorType2;

public:
	ScaleEstimateFilter();
	typedef itk::Vector<double, InputImageType1::ImageDimension> ScaleType;
	enum scaling { isotropic, anisotropic };
	void SetInputImage1(  const typename InputImageType1::Pointer img1 );
	void SetInputImage2(  const typename InputImageType2::Pointer img2 );

	void SetNumberOfThreads( const unsigned int& threads )
		{
			if( threads>0 ) { m_NumberOfThreads = threads; }
			else { m_NumberOfThreads = 1; }

	}

	ScaleType EstimateScaling( scaling, const bool biggestExtent = false );

private:
	typename OtsuCalculatorType1::Pointer m_OtsuCalculator1;
	typename OtsuCalculatorType1::Pointer m_OtsuCalculator2;

	typename GaussianFilterType1::Pointer m_Filter1X;
	typename GaussianFilterType1::Pointer m_Filter1Y;
	typename GaussianFilterType1::Pointer m_Filter1Z;

	typename GaussianFilterType2::Pointer m_Filter2X;
	typename GaussianFilterType2::Pointer m_Filter2Y;
	typename GaussianFilterType2::Pointer m_Filter2Z;

	typename MomentsCalculatorType1::Pointer m_MomentsCalculator1;
	typename MomentsCalculatorType2::Pointer m_MomentsCalculator2;

	typedef itk::Vector<double, InputImageType1::ImageDimension> VectorType;
	typedef itk::Vector<bool, InputImageType1::ImageDimension> SuspiciousType;

	unsigned int m_NumberOfThreads;

	typename InputImageType1::Pointer m_Image1;
	typename InputImageType2::Pointer m_Image2;
	typename InputImageType1::SizeType m_Size1;
	typename InputImageType2::SizeType m_Size2;
	typename InputImageType1::SpacingType m_Spacing1;
	typename InputImageType2::SpacingType m_Spacing2;

	typename InputImageType1::Pointer m_InternImage1;
	typename InputImageType2::Pointer m_InternImage2;

	ScaleType m_Scaling;
	VectorType m_Moment1;
	VectorType m_Moment2;
	VectorType m_Extent1;
	VectorType m_Extent2;
	SuspiciousType m_Suspicious1;
	SuspiciousType m_Suspicious2;

	void filter();

};
} //end namespace extitk
} //end namsepace isis

#include "isisScaleEstimateFilter.cpp"

#endif /* ISISSCALEESTIMATEFILTER_HPP_ */

