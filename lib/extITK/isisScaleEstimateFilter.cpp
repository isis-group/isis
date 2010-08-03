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
	: m_NumberOfThreads(1)
{
	m_OtsuCalculator1 = OtsuCalculatorType1::New();
	m_OtsuCalculator2 = OtsuCalculatorType2::New();

	m_Filter1X = GaussianFilterType1::New();
	m_Filter1Y = GaussianFilterType1::New();
	m_Filter1Z = GaussianFilterType1::New();

	m_Filter2X = GaussianFilterType2::New();
	m_Filter2Y = GaussianFilterType2::New();
	m_Filter2Z = GaussianFilterType2::New();

	m_MomentsCalculator1 = MomentsCalculatorType1::New();
	m_MomentsCalculator2 = MomentsCalculatorType2::New();

	m_Suspicious1.Fill(false);
	m_Suspicious2.Fill(false);

	m_InternImage1 = InputImageType1::New();
	m_InternImage2 = InputImageType2::New();
	m_Scaling.Fill(1.0);
}


template<class InputImageType1, class InputImageType2>
void ScaleEstimateFilter<InputImageType1, InputImageType2>::SetInputImage1( const typename InputImageType1::Pointer img1 )
{
	m_Image1 = img1;
	m_Size1 = m_Image1->GetBufferedRegion().GetSize();
	m_Spacing1 = m_Image1->GetSpacing();

}
template<class InputImageType1, class InputImageType2>
void ScaleEstimateFilter<InputImageType1, InputImageType2>::SetInputImage2(  const typename InputImageType2::Pointer img2 )
{
	m_Image2 = img2;
	m_Size2 = m_Image2->GetBufferedRegion().GetSize();
	m_Spacing2 = m_Image2->GetSpacing();
}


template<class InputImageType1, class InputImageType2>
typename ScaleEstimateFilter<InputImageType1, InputImageType2>::ScaleType
ScaleEstimateFilter<InputImageType1, InputImageType2>::EstimateScaling( scaling myScalingType, const bool biggestExtent )
{
	if(!m_Image1 or !m_Image2) {
		std::cerr << "At least one of the two input images are not set. Returning isotropic scaling 1!";
		return m_Scaling;
	}
	if (m_Image1 and m_Image2)
	{
		filter();
		m_OtsuCalculator1->SetImage( m_InternImage1 );
		m_OtsuCalculator1->Compute();
		m_OtsuCalculator2->SetImage( m_InternImage2 );
		m_OtsuCalculator2->Compute();
		m_MomentsCalculator1->SetImage( m_InternImage1 );
		m_MomentsCalculator2->SetImage( m_InternImage2 );
		m_MomentsCalculator1->Compute();
		m_MomentsCalculator2->Compute();
		m_Moment1 = m_MomentsCalculator1->GetCenterOfGravity();
		m_Moment2 = m_MomentsCalculator2->GetCenterOfGravity();
		typename InputImageType1::PointType point1;
		typename InputImageType2::PointType point2;
		typename InputImageType1::IndexType indexMoment1;
		typename InputImageType2::IndexType indexMoment2;
		typename InputImageType1::IndexType indexIter1;
		typename InputImageType2::IndexType indexIter2;
		for (size_t i=0; i<InputImageType1::ImageDimension; i++ )
		{
			point1[i] = m_Moment1[i];
			point2[i] = m_Moment2[i];
		}
		m_InternImage1->TransformPhysicalPointToIndex( point1, indexMoment1 );
		m_InternImage2->TransformPhysicalPointToIndex( point2, indexMoment2 );
		indexIter1 = indexMoment1;
		indexIter2 = indexMoment2;
		//image1
		for ( size_t direction=0; direction<InputImageType1::ImageDimension; direction++ )
		{
			indexIter1 = indexMoment1;
			indexIter1[direction] = 0;
			while( m_InternImage1->GetPixel(indexIter1) < m_OtsuCalculator1->GetThreshold() && indexIter1[direction] < indexMoment1[direction] )
			{
				indexIter1[direction]++;
			}
			if(indexIter1[direction] == 0)
			{
				m_Suspicious1[direction] = true;
//				std::cout << "image 1, direction " << direction << " is suspicious[negative]";
			}
			m_Extent1[direction] = abs( indexIter1[direction] - indexMoment1[direction] );
			//other direction
			indexIter1 = indexMoment1;
			indexIter1[direction] = m_Size1[direction];
			while( m_InternImage1->GetPixel(indexIter1) < m_OtsuCalculator1->GetThreshold() && indexIter1[direction] >= indexMoment1[direction] )
			{
				indexIter1[direction]--;
			}
			if(static_cast<unsigned int>(indexIter1[direction]) == m_Size1[direction])
			{
				m_Suspicious1[direction] = true;
//				std::cout << "image 1, direction " << direction << " is suspicious[positive]";
			}
			m_Extent1[direction] += abs( indexIter1[direction] - indexMoment1[direction] );
//			std::cout << "image 1, direction " << direction << ": " << m_Extent1[direction] << std::endl;
		}
		//image2
		for ( size_t direction=0; direction<InputImageType2::ImageDimension; direction++ )
		{
			indexIter2 = indexMoment2;
			indexIter2[direction] = 0;
			while( m_InternImage2->GetPixel(indexIter2) < m_OtsuCalculator2->GetThreshold() && indexIter2[direction] < indexMoment2[direction] )
			{
				indexIter2[direction]++;
			}
			if(indexIter2[direction] == 0)
			{
				m_Suspicious2[direction] = true;
//				std::cout << "image 2, direction " << direction << " is suspicious[negative]";
			}
			m_Extent2[direction] = abs( indexIter2[direction] - indexMoment2[direction] );
			indexIter2 = indexMoment2;
			indexIter2[direction] = m_Size2[direction];
			while( m_InternImage2->GetPixel(indexIter2) < m_OtsuCalculator2->GetThreshold() && indexIter2[direction] >= indexMoment2[direction] )
			{
				indexIter2[direction]--;
			}
			if(static_cast<unsigned int>(indexIter2[direction]) == m_Size2[direction])
			{
				m_Suspicious2[direction] = true;
//				std::cout << "image 2, direction " << direction << " is suspicious[positive]";
			}
			m_Extent2[direction] += abs( indexIter2[direction] - indexMoment2[direction] );
//			std::cout << "image 2, direction " << direction << ": " << m_Extent2[direction] << std::endl;
			m_Scaling[direction] = ((float)m_Extent1[direction] * m_Spacing1[direction]) / ((float)m_Extent2[direction] * m_Spacing2[direction]);
//			std::cout << (float)m_Extent1[direction] << "*" << m_Spacing1[direction]
//			                              << "/" << (float)m_Extent2[direction] << "*" << m_Spacing2[direction]
//			                               << "=" << m_Scaling[direction] << std::endl;
		}
		if( myScalingType == isotropic )
		{
			float isoScaling=0;
			size_t notSuspiciousCounter=0;
			for ( size_t dimension = 0; dimension < InputImageType1::ImageDimension; dimension++)
			{
				if ( !m_Suspicious1[dimension] && !m_Suspicious2[dimension] )
				{
					notSuspiciousCounter++;
					isoScaling+=m_Scaling[dimension];
				}
			}
			m_Scaling.Fill(isoScaling / notSuspiciousCounter);
		}
		return m_Scaling;
	}
	return m_Scaling;

}

template<class InputImageType1, class InputImageType2>
void ScaleEstimateFilter<InputImageType1, InputImageType2>::filter()
{
	m_Filter1X->SetNumberOfThreads(m_NumberOfThreads);
	m_Filter1Y->SetNumberOfThreads(m_NumberOfThreads);
	m_Filter1Z->SetNumberOfThreads(m_NumberOfThreads);
	m_Filter2X->SetNumberOfThreads(m_NumberOfThreads);
	m_Filter2Y->SetNumberOfThreads(m_NumberOfThreads);
	m_Filter2Z->SetNumberOfThreads(m_NumberOfThreads);

	m_Filter1X->SetNormalizeAcrossScale(false);
	m_Filter1Y->SetNormalizeAcrossScale(false);
	m_Filter1Z->SetNormalizeAcrossScale(false);
	m_Filter1X->SetSigma(3);
	m_Filter1Y->SetSigma(3);
	m_Filter1Z->SetSigma(3);
	m_Filter1X->SetOrder( GaussianFilterType1::ZeroOrder );
	m_Filter1Y->SetOrder( GaussianFilterType1::ZeroOrder );
	m_Filter1Z->SetOrder( GaussianFilterType1::ZeroOrder );
	m_Filter1X->SetDirection(0);
	m_Filter1Y->SetDirection(1);
	m_Filter1Z->SetDirection(2);
	m_Filter1X->SetInput( m_Image1 );
	m_Filter1Y->SetInput( m_Filter1X->GetOutput() );
	m_Filter1Z->SetInput( m_Filter1Y->GetOutput() );
	m_Filter1Z->Update();
	m_InternImage1 = m_Filter1Z->GetOutput();

	m_Filter2X->SetNormalizeAcrossScale(false);
	m_Filter2Y->SetNormalizeAcrossScale(false);
	m_Filter2Z->SetNormalizeAcrossScale(false);
	m_Filter2X->SetSigma(3);
	m_Filter2Y->SetSigma(3);
	m_Filter2Z->SetSigma(3);
	m_Filter2X->SetOrder( GaussianFilterType2::ZeroOrder );
	m_Filter2Y->SetOrder( GaussianFilterType2::ZeroOrder );
	m_Filter2Z->SetOrder( GaussianFilterType2::ZeroOrder );
	m_Filter2X->SetDirection(0);
	m_Filter2Y->SetDirection(1);
	m_Filter2Z->SetDirection(2);
	m_Filter2X->SetInput( m_Image2 );
	m_Filter2Y->SetInput( m_Filter2X->GetOutput() );
	m_Filter2Z->SetInput( m_Filter2Y->GetOutput() );
	m_Filter2Z->Update();
	m_InternImage2 = m_Filter2Z->GetOutput();

}

} //end namespace extitk
} //end namespace isis
