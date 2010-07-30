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


#include "itkOtsuThresholdImageCalculator.h"
#include "itkImageSliceIteratorWithIndex.h"

#include <boost/foreach.hpp>

namespace isis {

namespace extitk{


template<class InputImageType1, class InputImageType2>
class ScaleEstimateFilter
{
	typedef itk::Vector<double, InputImageType1::ImageDimension> ScaleType;
	typedef itk::OtsuThresholdImageCalculator<InputImageType1> OtsuCalculatorType1;
	typedef itk::OtsuThresholdImageCalculator<InputImageType2> OtsuCalculatorType2;

	typedef itk::ImageSliceIteratorWithIndex<InputImageType1> IteratorType1;
	typedef itk::ImageSliceIteratorWithIndex<InputImageType2> IteratorType2;

public:
	ScaleEstimateFilter();
	enum scaling { isotropic, anisotropic };
	void SetInputImage1(  const typename InputImageType1::Pointer img1 );
	void SetInputImage2(  const typename InputImageType2::Pointer img2 );

	ScaleType EstimateScaling( scaling );



private:
	typename OtsuCalculatorType1::Pointer m_OtsuCalculator1;
	typename OtsuCalculatorType1::Pointer m_OtsuCalculator2;
	IteratorType1 m_Iterator1;
	IteratorType2 m_Iterator2;
	typename InputImageType1::Pointer m_Image1;
	typename InputImageType2::Pointer m_Image2;
	ScaleType m_Extent1;
	ScaleType m_Extent2;
	ScaleType m_Scaling;

};
} //end namespace extitk
} //end namsepace isis

#include "isisScaleEstimateFilter.cpp"

#endif /* ISISSCALEESTIMATEFILTER_HPP_ */

