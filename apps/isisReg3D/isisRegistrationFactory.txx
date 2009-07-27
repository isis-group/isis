/*
 * isisRegistrationFactory.txx
 *
 *  Created on: Jul 13, 2009
 *      Author: tuerke
 */

#include "isisRegistrationFactory.h"

namespace isis {


template< class TFixedImageType, class TMovingImageType >
RegistrationFactory< TFixedImageType, TMovingImageType >
::RegistrationFactory()
{
	m_FixedImageDimension = TFixedImageType::ImageDimension;
	m_MovingImageDimension = TMovingImageType::ImageDimension;

	m_RegistrationObject = RegistrationMethodType::New();
}


template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory< TFixedImageType, TMovingImageType >
::SetInterpolationType( eInterpolationType e_interpolator )
{
	switch (e_interpolator)
	{
	case Linear:
		std::cout << "Using the linear interpolation method!" << std::endl;
		typedef itk::LinearInterpolateImageFunction< TMovingImageType, double >
			InterpolatorType;
		typename InterpolatorType::Pointer interpolator = InterpolatorType::New();

		m_RegistrationObject->SetInterpolator( interpolator );

		break;


	}

}


template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory< TFixedImageType, TMovingImageType >
::SetTransformType( eTransformType e_transform )
{
	switch (e_transform)
	{
	case Rigid:
		if (m_FixedImageDimension == 2 and m_MovingImageDimension == 2)
		{
			typedef itk::Rigid2DTransform< double > TransformType;
			TransformType::Pointer transform = TransformType::New();
			m_RegistrationObject->SetTransform( transform );
			break;
		}
		if (m_FixedImageDimension == 3 and m_MovingImageDimension == 3)
		{
			typedef itk::Rigid3DTransform< double > TransformType;
			TransformType::Pointer transform = TransformType::New();
			m_RegistrationObject->SetTransform( transform );
			break;
		}
	case RigidVersor3D:
		if (m_FixedImageDimension == 3 and m_MovingImageDimension == 3)
		{
			typedef itk::VersorRigid3DTransform< double > TransformType;
			TransformType::Pointer transform = TransformType::New();
			m_RegistrationObject->SetTransform( transform );
			break;
		}
		else
		{
			std::cout << "Dimension has to be 3 to use VersorRigid3DTransform!!" << std::endl;
			break;
		}


	}
}


template< class TFixedImageType, class TMovingImageType >
typename RegistrationFactory< TFixedImageType, TMovingImageType >::RegistrationMethodType
RegistrationFactory< TFixedImageType, TMovingImageType >
::GetRegistrationObject( void )
{
	return m_RegistrationObject;
}





} //end namespace isis
