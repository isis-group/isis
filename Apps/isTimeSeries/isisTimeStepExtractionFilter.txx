/*
 * isisTimeStepExtractionFilter.txx
 *
 *  Created on: May 15, 2009
 *      Author: tuerke
 */
#ifndef __TIMESTEPEXTRACTIONFILTER_TXX
#define __TIMESTEPEXTRACTIONFILTER_TXX

#include "isisTimeStepExtractionFilter.h"

namespace isis {

template< class TInputImage, class TOutputImage >
TimeStepExtractionFilter< TInputImage, TOutputImage >
::TimeStepExtractionFilter()
{
	m_ExtractFilter = ExtractFilterType::New();
	m_RequestedTimeStep = 1;

}



template< class TInputImage, class TOutputImage >
void
TimeStepExtractionFilter< TInputImage, TOutputImage>
::GenerateData()
{
	m_InputRegion = this->GetInput()->GetLargestPossibleRegion();
	m_InputSize = m_InputRegion.GetSize();
	m_InputIndex = m_InputRegion.GetIndex();

	m_InputSize[ OutputImageDimension ] = 0;
	m_InputIndex[ OutputImageDimension ] = m_RequestedTimeStep;

	m_DesiredRegion.SetSize( m_InputSize );
	m_DesiredRegion.SetIndex( m_InputIndex );

	m_ExtractFilter->SetExtractionRegion( m_DesiredRegion );
	m_ExtractFilter->SetInput( this->GetInput() );
	try
		{
			m_ExtractFilter->Update();
		}
		catch( itk::ExceptionObject & err )
		{
			std::cerr << "ExceptionObject caught ! GNA!" << std::endl;
			std::cerr << err << std::endl;
		}


	this->GraftOutput( m_ExtractFilter->GetOutput() );

}

/*
template< class TInputImage, class TOutputImage >
void
TimeStepExtractorFilter< TInputImage, TOutputImage >
::Start( void )
{
	typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();



	//this collapses the last dimension


	extractFilter->SetExtractionRegion( m_OutputRegion );
	extractFilter->SetInput( m_InputImage );
	try
	{
		extractFilter->Update();
	}
	catch( itk::ExceptionObject & err )
	{
		std::cerr << "ExceptionObject caught !" << std::endl;
		std::cerr << err << std::endl;
	}
	m_OutputImage = extractFilter->GetOutput();
}

*/
}// end namspace isis
#endif
