/*
 * isisTimeStepExtractionFilter.txx
 *
 *  Created on: May 15, 2009
 *      Author: tuerke
 */
#ifndef __TIMESTEPEXTRACTIONFILTER_TXX
#define __TIMESTEPEXTRACTIONFILTER_TXX

#include "isisTimeStepExtractionFilter.h"
#include "itkImageBase.h"

namespace isis {

template<class TInputImage, class TOutputImage>
TimeStepExtractionFilter<TInputImage, TOutputImage>::TimeStepExtractionFilter() {
	m_ExtractFilter = ExtractFilterType::New();
	m_RequestedTimeStep = 0;
	m_RequestedTimeRangeBegin = 0;
	m_RequestedTimeRangeEnd = 0;

}

template<class TInputImage, class TOutputImage>
void TimeStepExtractionFilter<TInputImage, TOutputImage>::Update() {

	if(!m_RequestedTimeRangeBegin and !m_RequestedTimeRangeEnd) {

		//extraction of a single timestep if m_RequestedTimeRangeBegin and m_RequestedTimeRangeEnd not set
		m_InputImageRegion = this->GetInput()->GetLargestPossibleRegion();
		m_InputSize = m_InputImageRegion.GetSize();
		m_InputIndex = m_InputImageRegion.GetIndex();
		//this collapses the last dimension so the output dimension is input dimension-1
		m_InputSize[OutputImageDimension] = 0;
		m_InputIndex[OutputImageDimension] = m_RequestedTimeStep;
		m_DesiredRegion.SetSize(m_InputSize);
		m_DesiredRegion.SetIndex(m_InputIndex);
		m_ExtractFilter->Modified();
		m_ExtractFilter->SetExtractionRegion(m_DesiredRegion);
		m_ExtractFilter->SetInput(this->GetInput());

		try {
			m_ExtractFilter->Update();
		} catch(itk::ExceptionObject & err) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;

		}

		m_OutputImage = m_ExtractFilter->GetOutput();

	} else {
		if(m_RequestedTimeRangeEnd - m_RequestedTimeRangeBegin > 1) {
			//extraction of a certain timespan set by m_RequestedTimeRangeBegin and m_RequestedTimeRangeBegin
			//notice that the timespan has to be bigger than 1
			std::cout << "extracting time range " << m_RequestedTimeRangeBegin << " to " << m_RequestedTimeRangeEnd
			        << std::endl;
			m_InputImageRegion = this->GetInput()->GetLargestPossibleRegion();
			m_InputSize = m_InputImageRegion.GetSize();
			m_InputIndex = m_InputImageRegion.GetIndex();

			m_InputSize[OutputImageDimension - 1] = m_RequestedTimeRangeEnd - m_RequestedTimeRangeBegin;
			m_InputIndex[OutputImageDimension - 1] = m_RequestedTimeRangeBegin;

			m_DesiredRegion.SetSize(m_InputSize);
			m_DesiredRegion.SetIndex(m_InputIndex);

			m_ExtractFilter->SetExtractionRegion(m_DesiredRegion);
			m_ExtractFilter->SetInput(this->GetInput());
			try {
				m_ExtractFilter->Update();
			} catch(itk::ExceptionObject & err) {
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
typename TimeStepExtractionFilter<TInputImage, TOutputImage>::OutputImageType::Pointer TimeStepExtractionFilter<
        TInputImage, TOutputImage>::GetOutput() const {
	return m_OutputImage;
}

}// end namspace isis
#endif
