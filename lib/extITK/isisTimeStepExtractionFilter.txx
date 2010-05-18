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


#ifndef __TIMESTEPEXTRACTIONFILTER_TXX
#define __TIMESTEPEXTRACTIONFILTER_TXX

#include "isisTimeStepExtractionFilter.hpp"
#include "itkImageBase.h"
#include <iterator>

namespace isis
{
namespace extitk
{

template<class TInputImage, class TOutputImage>
TimeStepExtractionFilter<TInputImage, TOutputImage>::TimeStepExtractionFilter()
{
	m_ExtractFilter = ExtractFilterType::New();
	m_RequestedTimeStep = 0;
	m_RequestedTimeRangeBegin = 0;
	m_RequestedTimeRangeEnd = 0;
}

template<class TInputImage, class TOutputImage>
void TimeStepExtractionFilter<TInputImage, TOutputImage>::Update()
{
	if ( !m_RequestedTimeRangeBegin and !m_RequestedTimeRangeEnd ) {
		//extraction of a single timestep if m_RequestedTimeRangeBegin and m_RequestedTimeRangeEnd not set
		m_InputImageRegion = this->GetInput()->GetLargestPossibleRegion();
		m_InputSize = m_InputImageRegion.GetSize();
		m_InputIndex = m_InputImageRegion.GetIndex();
		//this collapses the last dimension so the output dimension is input dimension-1
		m_InputSize[OutputImageDimension] = 0;
		m_InputIndex[OutputImageDimension] = m_RequestedTimeStep;
		m_DesiredRegion.SetSize( m_InputSize );
		m_DesiredRegion.SetIndex( m_InputIndex );
		m_ExtractFilter->Modified();
		m_ExtractFilter->SetExtractionRegion( m_DesiredRegion );
		m_ExtractFilter->SetInput( this->GetInput() );

		try {
			m_ExtractFilter->Update();
		} catch ( itk::ExceptionObject &err ) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
		}

		m_OutputImage = m_ExtractFilter->GetOutput();
	} else {
		if ( m_RequestedTimeRangeEnd - m_RequestedTimeRangeBegin > 1 ) {
			//extraction of a certain timespan set by m_RequestedTimeRangeBegin and m_RequestedTimeRangeBegin
			//notice that the timespan has to be bigger than 1
			std::cout << "extracting time range " << m_RequestedTimeRangeBegin << " to " << m_RequestedTimeRangeEnd
					  << std::endl;
			m_InputImageRegion = this->GetInput()->GetLargestPossibleRegion();
			m_InputSize = m_InputImageRegion.GetSize();
			m_InputIndex = m_InputImageRegion.GetIndex();
			m_InputSize[OutputImageDimension - 1] = m_RequestedTimeRangeEnd - m_RequestedTimeRangeBegin;
			m_InputIndex[OutputImageDimension - 1] = m_RequestedTimeRangeBegin;
			m_DesiredRegion.SetSize( m_InputSize );
			m_DesiredRegion.SetIndex( m_InputIndex );
			m_ExtractFilter->SetExtractionRegion( m_DesiredRegion );
			m_ExtractFilter->SetInput( this->GetInput() );

			try {
				m_ExtractFilter->Update();
			} catch ( itk::ExceptionObject &err ) {
				std::cerr << "ExceptionObject caught !" << std::endl;
				std::cerr << err << std::endl;
			}

			m_OutputImage = m_ExtractFilter->GetOutput();
		} else {
			std::cout << "range has to be bigger than 1" << std::endl;
		}
	}
}

template<class TInputImage, class TOutputImage>
typename TimeStepExtractionFilter<TInputImage, TOutputImage>::OutputImageType::Pointer TimeStepExtractionFilter <
TInputImage, TOutputImage >::GetOutput() const
{
	return m_OutputImage;
}

} // end namespace extitk
}// end namspace isis
#endif
