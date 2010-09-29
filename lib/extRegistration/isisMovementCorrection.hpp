/*
 * isisMovementCorrection.hpp
 *
 *  Created on: Aug 19, 2010
 *      Author: tuerke
 */

#ifndef ISISMOVEMENTCORRECTION_HPP_
#define ISISMOVEMENTCORRECTION_HPP_

#include "itkImage.h"
#include "isisTimeStepExtractionFilter.hpp"
#include "isisRegistrationFactory3D.hpp"

namespace isis {
namespace registration {


template<class TYPE>
class MovementCorrectionFilter
{
private:
	typedef TYPE InputImageType;
	typedef itk::Image<typename TYPE::PixelType, 3> InternImageType;
	typedef isis::registration::RegistrationFactory3D< InternImageType, InternImageType > RegistrationFactoryType;
	typedef isis::extitk::TimeStepExtractionFilter<InputImageType, InternImageType> ExtractorType;

	typename InputImageType::Pointer m_InputImage;
	RegistrationFactoryType::Pointer m_RegistrationFactory = RegistrationFactory3D::New();
	ExtractorType* m_Extractor;


public:
	void setInputImage( const typename TYPE::Pointer img) { m_InputImage = img; }
	void correct( void );

};





}
}

#endif /* ISISMOVEMENTCORRECTION_HPP_ */
