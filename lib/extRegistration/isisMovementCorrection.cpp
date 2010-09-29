/*
 * isisMovementCorrection.cpp
 *
 *  Created on: Aug 19, 2010
 *      Author: tuerke
 */

#include "isisMovementCorrection.hpp"

namespace isis {
namespace registration {

template<class TYPE>
void MovementCorrectionFilter<TYPE>::MovementCorrectionFilter()
{
	m_Extractor = ExtractorType::New();
	m_RegistrationFactory = RegistrationFactory3D::New();
}



template<class TYPE>
void MovementCorrectionFilter<TYPE>::correct()
{
	m_Extractor->SetInput( m_InputImage );
	InternImageType::Pointer tmpImage = InternImageType::New();
	for( size_t timestep = 1; timestep < m_InputImage->GetLargestPossibleRegion().GetSize[3]; timestep++ )
	{
		m_Extractor->SetRequestedTimeStep(timestep);
		m_Extractor->Update();
		tmpImage = m_Extractor->GetOutput();
	}
}

}
}
