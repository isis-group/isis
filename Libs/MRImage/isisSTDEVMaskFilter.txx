/*
 * isisSTDEVMaskFilter.txx
 *
 *  Created on: Jun 23, 2009
 *      Author: tuerke
 */

#include "isisSTDEVMaskFilter.h"

namespace isis {


template< class TInputImage, class TOutputImage >
STDEVMaskFilter< TInputImage, TOutputImage >
::STDEVMaskFilter()
{
	m_InputImage = InputImageType::New();
	m_OutputImage = OutputImageType::New();
}


template< class TInputImage, class TOutputImage >
void
STDEVMaskFilter< TInputImage, TOutputImage >
::SetOutputParameters()
{
	m_InputRegion = m_InputImage->GetBufferedRegion();

	m_InputIndex = m_InputRegion.GetIndex();
	m_InputSize = m_InputRegion.GetSize();
	m_InputSpacing = m_InputImage->GetSpacing();
	m_InputOrigin = m_InputImage->GetOrigin();

	for( unsigned int i = 0; i < OutputImageDimension; i++ )
	{
		m_OutputSize[i] = m_InputSize[i];
		m_OutputIndex[i] = m_InputIndex[i];
		m_OutputSpacing[i] = m_InputSpacing[i];
		m_OutputOrigin[i] = m_InputOrigin[i];
	}

	m_OutputImage->SetSpacing( m_OutputSpacing );
	m_OutputImage->SetOrigin( m_OutputOrigin );
	m_OutputRegion.SetSize( m_OutputSize );
	m_OutputRegion.SetIndex( m_OutputIndex );

	m_OutputImage->SetRegions( m_OutputRegion );
	m_OutputImage->Allocate();

	m_Timelength = m_InputRegion.GetSize()[OutputImageDimension];



}


} //end namespace isis
