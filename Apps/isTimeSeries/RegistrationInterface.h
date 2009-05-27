/*
 * RegistrationInterface.h
 *
 *  Created on: May 26, 2009
 *      Author: tuerke
 */

#ifndef REGISTRATIONINTERFACE_H_
#define REGISTRATIONINTERFACE_H_

//registration related files

#include "itkImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRegularStepGradientDescentOptimizer.h"

//transform types
#include "itkTranslationTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkScaleSkewVersor3DTransform.h"
#include "itkBSplineDeformableTransform.h"

//metric types
#include "itkMutualInformationImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"

//optimizer types
#include "itkGradientDescentOptimizer.h"
#include "itkOnePlusOneEvolutionaryOptimizer.h"

//interpolator types
#include "itkLinearInterpolateImageFunction.h"




template< class TFixedImageType, class TMovingImageType >
class RegistrationInterface : public itk::ImageRegistrationMethod < TFixedImageType, TMovingImageType>
{

public:
	RegistrationInterface();

	typedef RegistrationInterface						Self;
	typedef TFixedImageType								FixedImageType;
	typedef TMovingImageType							MovingImageType;

	typedef typename FixedImageType::SizeType			FixedImageSizeType;
	typedef typename MovingImageType::SizeType			MovingImageSizeType;

	typedef typename FixedImageType::RegionType			FixedRegionType;
	typedef typename MovingImageType::RegionType		MovingRegionType;

	typedef typename FixedImageType::Pointer			FixedImagePointer;
	typedef typename MovingImageType::Pointer 			MovingImagePointer;

	typedef typename itk::ImageRegistrationMethod
	< FixedImageType, MovingImageType > 				ImageRegistrationMethodType;

	//transform types
	typedef typename itk::TranslationTransform
	< double, 3 >										TranslationTransformType;
	typedef typename itk::VersorRigid3DTransform
	< double >											RigidTransformType;
	typedef typename itk::ScaleSkewVersor3DTransform
	< double >											AffineTransformType;
	typedef typename itk::BSplineDeformableTransform
	< double, 3, 3 > 									DeformableTransformType;

	//optimizer types
	typedef typename itk::GradientDescentOptimizer		GradientOptimizerType;
	typedef typename itk::OnePlusOneEvolutionaryOptimizer EvolutionOptimizerType;

	//metric types
	typedef typename itk::MutualInformationImageToImageMetric
	< FixedImageType, MovingImageType> 					MutualInformationType;
	typedef typename itk::MattesMutualInformationImageToImageMetric
	< FixedImageType, MovingImageType > 				MattesMutualInformationType;
	typedef typename itk::MeanSquaresImageToImageMetric
	< FixedImageType, MovingImageType >					MeanSquareMetricType;

	//interpolator types
	typedef typename itk::LinearInterpolateImageFunction
	< MovingImageType, double > 						LinearInterpolatorType;
	typedef enum MetricType
	{
		MEANSQUARE,
		MUTUALINFORMATION,
		MATTESMUTUALINFORMATION
	};
	typedef enum RegistrationMethodType
	{
		TRANSLATION,
		RIGID,
		AFFINE,
		NONLINEAR
	};

	typedef enum OptimizerType
	{
		GRADIENTDESCENT,
		ONEPLUSONEEVOLUTION
	};

	typedef enum InterpolatorType
	{
		LINEAR
	};

	void StartRegistration( void );


	//setter methods
	void SetFixedImage( FixedImagePointer );
	void SetMovingImage( MovingImagePointer );
	void SetMetric( MetricType );
	void SetRegistrationMethod( RegistrationMethodType );
	void SetOptimizer( OptimizerType );

protected:
	virtual void Initialize( void ) throw ( itk::ExceptionObject );
	void SetUpOptimizer( void );


private:
	unsigned int									m_NumberOfHistogramBins;
	unsigned int 									m_NumberOfSpatialSamples;
	unsigned int 									m_NumberOfFixedImagePixels;

	bool											m_FixedImageIsBigger;

	MetricType 										m_Metric;
	RegistrationMethodType 							m_RegistrationMethod;
	OptimizerType 									m_Optimizer;
	InterpolatorType 								m_Interpolator;

	FixedImagePointer 								m_FixedImage;
	MovingImagePointer								m_MovingImage;

	FixedRegionType									m_FixedRegion;
	MovingRegionType								m_MovingRegion;

	FixedImageSizeType								m_FixedImageSize;
	MovingImageSizeType								m_MovingImageSize;

	typename ImageRegistrationMethodType::Pointer	m_ImageRegistration;

	//tansform types
	typename TranslationTransformType::Pointer		m_TranslationTransform;
	typename RigidTransformType::Pointer			m_RigidTransform;
	typename AffineTransformType::Pointer			m_AffineTransform;
	typename DeformableTransformType::Pointer		m_DeformableTransform;


	//optimizer types
	typename GradientOptimizerType::Pointer			m_GradientOptimizer;
	typename EvolutionOptimizerType::Pointer		m_EvolutionOptimizer;


	//metric types
	typename MeanSquareMetricType::Pointer 			m_MeanSquareMetric;
	typename MutualInformationType::Pointer			m_MutualInformationMetric;
	typename MattesMutualInformationType::Pointer 	m_MattesMutualInformationMetric;


	//interpolator types
	typename LinearInterpolatorType::Pointer 		m_LinearInterpolator;
};




#include "RegistrationInterface.txx"
#endif /* REGISTRATIONINTERFACE_H_ */
