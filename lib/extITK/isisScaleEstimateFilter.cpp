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

#include "isisScaleEstimateFilter.hpp"

namespace isis {
namespace extitk {

template<class InputImageType1, class InputImageType2>
ScaleEstimateFilter<InputImageType1, InputImageType2>::ScaleEstimateFilter()
{
	m_OtsuCalculator1 = OtsuCalculatorType1::New();
	m_OtsuCalculator2 = OtsuCalculatorType2::New();
	m_Scaling.Fill(1.0);
}


template<class InputImageType1, class InputImageType2>
void ScaleEstimateFilter<InputImageType1, InputImageType2>::SetInputImage1( const typename InputImageType1::Pointer img1 )
{
	m_Image1 = img1;
	m_OtsuCalculator1->SetImage( m_Image1 );
	m_OtsuCalculator1->Compute();
	m_Iterator1 = IteratorType1(m_Image1, m_Image1->GetRequestedRegion() );
}
template<class InputImageType1, class InputImageType2>
void ScaleEstimateFilter<InputImageType1, InputImageType2>::SetInputImage2(  const typename InputImageType2::Pointer img2 )
{
	m_Image2 = img2;
	m_OtsuCalculator2->SetImage( m_Image2 );
	m_OtsuCalculator2->Compute();
	m_Iterator2 = IteratorType1(m_Image2, m_Image2->GetRequestedRegion() );
}


template<class InputImageType1, class InputImageType2>
typename ScaleEstimateFilter<InputImageType1, InputImageType2>::ScaleType ScaleEstimateFilter<InputImageType1, InputImageType2>::EstimateScaling( scaling myScalingType )
{
	if(!m_Image1 or !m_Image2) {
		std::cerr << "At least one of the two input images are not set. Returning isotropic scaling 1!";
		return m_Scaling;
	}
	if (m_Image1 and m_Image2)
	{
		std::vector<size_t> extents1;
		std::vector<size_t> extents2;
		for (size_t dimension=0; dimension < InputImageType1::ImageDimension; dimension++)
		{
			size_t currentSlice=0;
			bool reached=false;
			m_Iterator1.SetFirstDirection(dimension);
			m_Iterator1.SetSecondDirection( dimension+1 == InputImageType1::ImageDimension ? 0 : dimension+1 );
			m_Iterator1.GoToBegin();
			while( ! m_Iterator1.IsAtEnd() && !reached)
			{
				while( ! m_Iterator1.IsAtEndOfSlice() && !reached)
				{
					while( ! m_Iterator1.IsAtEndOfLine() && !reached)
					{
						++m_Iterator1;
						if( m_Iterator1.Value() >= m_OtsuCalculator1->GetThreshold() )
						{
							extents1.push_back(currentSlice);
							reached=true;
						}
					}
					m_Iterator1.NextLine();
				}
				currentSlice++;
				m_Iterator1.NextSlice();
			}
			currentSlice= m_Image1->GetBufferedRegion().GetSize()[((dimension+1 == InputImageType1::ImageDimension ? 0 : dimension+1) == InputImageType1::ImageDimension-1 ? 0 : (dimension+1 == InputImageType1::ImageDimension ? 0 : dimension+1)+1)];
			reached=false;
			m_Iterator1.GoToReverseBegin();
			while( ! m_Iterator1.IsAtReverseEnd() && !reached)
			{
				while( ! m_Iterator1.IsAtReverseEndOfSlice() && !reached)
				{
					while( ! m_Iterator1.IsAtReverseEndOfLine() && !reached)
					{
						--m_Iterator1;
						if( m_Iterator1.Value() >= m_OtsuCalculator1->GetThreshold() )
						{
							reached=true;
							extents1.push_back(currentSlice);
						}
					}
					m_Iterator1.PreviousLine();
				}
				currentSlice--;
				m_Iterator1.PreviousSlice();
			}
		}
		for (size_t dimension=0; dimension < InputImageType1::ImageDimension; dimension++)
		{
			size_t currentSlice=0;
			bool reached=false;
			m_Iterator2.SetFirstDirection(dimension);
			m_Iterator2.SetSecondDirection( dimension+1 == InputImageType1::ImageDimension ? 0 : dimension+1 );
			m_Iterator2.GoToBegin();
			while( ! m_Iterator2.IsAtEnd() && !reached)
			{
				while( ! m_Iterator2.IsAtEndOfSlice() && !reached)
				{
					while( ! m_Iterator2.IsAtEndOfLine() && !reached)
					{
						++m_Iterator2;
						if( m_Iterator2.Value() >=  m_OtsuCalculator2->GetThreshold() )
						{
							extents2.push_back(currentSlice);
							reached=true;
						}
					}
					m_Iterator2.NextLine();
				}
				currentSlice++;
				m_Iterator2.NextSlice();
			}
			currentSlice = m_Image2->GetBufferedRegion().GetSize()[((dimension+1 == InputImageType1::ImageDimension ? 0 : dimension+1) == InputImageType1::ImageDimension-1 ? 0 : (dimension+1 == InputImageType1::ImageDimension ? 0 : dimension+1)+1)];
			reached=false;
			m_Iterator2.GoToReverseBegin();
			while( ! m_Iterator2.IsAtReverseEnd() && !reached)
			{
				while( ! m_Iterator2.IsAtReverseEndOfSlice() && !reached)
				{
					while( ! m_Iterator2.IsAtReverseEndOfLine() && !reached)
					{
						--m_Iterator2;
						if( m_Iterator2.Value() >= m_OtsuCalculator2->GetThreshold() )
						{
							reached=true;
							extents2.push_back(currentSlice);
						}
					}
					m_Iterator2.PreviousLine();
				}
				currentSlice--;
				m_Iterator2.PreviousSlice();
			}
		}
		BOOST_FOREACH(std::vector<size_t>::const_reference ref, extents1)
		{
			std::cout << ref << std::endl;
		}
		BOOST_FOREACH(std::vector<size_t>::const_reference ref, extents2)
		{
			std::cout << ref << std::endl;
		}


	}

}



} //end namespace extitk
} //end namespace isis
