/*
 * isisRegistrationFactory.h
 *
 *  Created on: Jul 13, 2009
 *      Author: tuerke
 */

#ifndef ISISREGISTRATIONFACTORY_H_
#define ISISREGISTRATIONFACTORY_H_

#include "itkImageRegistrationMethod.h"


//transform includes
#include "itkVersorRigid3DTransform.h"
#include "itkRigid3DTransform.h"
#include "itkRigid2DTransform.h"

//metric includes
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedMutualInformationHistogramImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"

//optimizer inlcudes
#include "itkRegularStepGradientDescentOptimizer.h"

//interpolator includes
#include "itkLinearInterpolateImageFunction.h"


namespace isis {


template< class TFixedImageType, class TMovingImageType >
class RegistrationFactory
{
public:


	typedef RegistrationFactory						Self;

	typedef typename TFixedImageType::PixelType		FixedPixelType;
	typedef typename TMovingImageType::PixelType	MovingPixelType;

	typedef itk::ImageRegistrationMethod< TFixedImageType, TMovingImageType >
													RegistrationMethodType;





	enum eTransformType
	{
		RigidVersor3D,
		Rigid,
		Affine
	};

	enum eMetricType
	{
		MattesMutualInformation,
		NormalizedMattesMutualInformation,
		Correlation
	};

	enum eOptimizerType
	{
		RegularStepGradientDescent
	};

	enum eInterpolationType
	{
		Linear
	};




	//setter methods
	void SetTransformType( eTransformType );
	void SetMetricType( eMetricType );
	void SetOptimizerType( eOptimizerType );
	void SetInterpolationType( eInterpolationType );



	//getter methods
	RegistrationMethodType GetRegistrationObject( void );


	RegistrationFactory();
	virtual ~RegistrationFactory() {}

private:

	typename RegistrationMethodType::Pointer 							m_RegistrationObject;
	unsigned int														m_FixedImageDimension;
	unsigned int 														m_MovingImageDimension;
};



}
#include "isisRegistrationFactory.txx"


#endif /* ISISREGISTRATIONFACTORY_H_ */
