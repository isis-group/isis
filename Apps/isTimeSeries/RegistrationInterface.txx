/*
 * RegistrationInterface.txx
 *
 *  Created on: May 26, 2009
 *      Author: tuerke
 */

#include "RegistrationInterface.h"

namespace isis {

template< class TFixedImageType, class TMovingImageType >
RegistrationInterface< TFixedImageType, TMovingImageType >
::RegistrationInterface()
{
	m_NumberOfHistogramBins = 30;
	m_FixedImageIsBigger = false;
	m_ImageRegistration = ImageRegistrationMethodType::New();






}template< class TFixedImage, class TMovingImage >
void
RegistrationInterface< TFixedImage, TMovingImage >
::StartRegistration( void )
{
	this->Initialize();
}


template< class TFixedImage, class TMovingImage >
void
RegistrationInterface< TFixedImage, TMovingImage >
::Initialize() throw ( itk::ExceptionObject )
{
	//setup the registration method
	switch ( m_RegistrationMethod )
	{
	default:
		case TRANSLATION:
		{
			std::cout << "Using a translation transform" << std::endl;
			m_TranslationTransform = TranslationTransformType::New();
			m_ImageRegistration->SetTransform( m_TranslationTransform );
			break;
		}
		case RIGID:
		{
			std::cout << "Using a rigid trasform" << std::endl;
			m_RigidTransform = RigidTransformType::New();
			m_ImageRegistration->SetTransform( m_RigidTransform );
			break;
		}
		case AFFINE:
		{
			std::cout << "using an affine transform" << std::endl;
			m_AffineTransform = AffineTransformType::New();
			m_ImageRegistration->SetTransform( m_AffineTransform );
			break;
		}
		case NONLINEAR:
		{
			std::cout << "using a nonlinear transform" << std::endl;
			m_DeformableTransform = DeformableTransformType::New();
			m_ImageRegistration->SetTransform( m_DeformableTransform );
			break;
		}

	}
	//setup the optimizer
	switch ( m_Optimizer )
	{
	default:
		case GRADIENTDESCENT:
		{
			std::cout << "using a gradient descent optimizer" << std::endl;
			m_GradientOptimizer = GradientOptimizerType::New();
			m_ImageRegistration->SetOptimizer( m_GradientOptimizer );
			break;
		}
		case ONEPLUSONEEVOLUTION:
			std::cout << "using a OnePlusOneEvolutionOptimizer" << std::endl;
			m_EvolutionOptimizer = EvolutionOptimizerType::New();
			m_ImageRegistration->SetOptimizer( m_EvolutionOptimizer );
			break;
	}
	//setup the metric
	switch( m_Metric )
	{
	default:
		case MEANSQUARE:
		{
			std::cout << "using a mean square metric" << std::endl;
			m_MeanSquareMetric = MeanSquareMetricType::New();
			m_ImageRegistration->SetMetric( m_MeanSquareMetric );
			break;
		}
		case MUTUALINFORMATION:
		{
			std::cout << "using a mutual information metric" << std::endl;
			m_MutualInformationMetric = MutualInformationType::New();
			if (!m_FixedImageIsBigger)
				{
					m_MutualInformationMetric->SetNumberOfSpatialSamples( static_cast<int>( m_NumberOfFixedImagePixels * 0.01 ) );
				}
			m_MattesMutualInformationMetric->SetNumberOfHistogramBins( m_NumberOfHistogramBins );
			m_ImageRegistration->SetMetric( m_MutualInformationMetric );
			break;
		}
		case MATTESMUTUALINFORMATION:
		{
			std::cout << "using a mattes mutual information metric" << std::endl;
			m_MattesMutualInformationMetric = MattesMutualInformationType::New();
			m_MattesMutualInformationMetric->SetNumberOfHistogramBins( m_NumberOfHistogramBins );

			m_ImageRegistration->SetMetric( m_MattesMutualInformationMetric );
			break;
		}

	}
	//setup the interpolator
	switch( m_Interpolator )
	{
	default:
		case LINEAR:
		{
			std::cout << "using a linear interpolator" << std::endl;
			m_LinearInterpolator = LinearInterpolatorType::New();
			m_ImageRegistration->SetInterpolator( m_LinearInterpolator );
			break;

		}
	}

	//setting up fixed and moving image
	m_ImageRegistration->SetFixedImage( m_FixedImage );
	m_ImageRegistration->SetMovingImage( m_MovingImage );

	//check whether fixed image is bigger than moving image or not
	for (int i = 0; i < 3; i++)
	{
	    if (m_FixedImageSize[i] > m_MovingImageSize[i])
	    {
	    	m_FixedImageIsBigger = true;
	    }
	}


	this->SetUpOptimizer();



}









template< class TFixedImageType, class TMovingImageType >
void
RegistrationInterface< TFixedImageType, TMovingImageType >
::SetUpOptimizer( void )
{

}











template< class TFixedImageType, class TMovingImageType >
void
RegistrationInterface< TFixedImageType, TMovingImageType >
::SetFixedImage( Self::FixedImagePointer fixedImage )
{
	m_FixedImage = fixedImage;
	m_FixedRegion = m_FixedImage->GetLargestPossibleRegion();
	m_NumberOfFixedImagePixels = m_FixedRegion.GetNumberOfPixels();
	m_FixedImageSize = m_FixedRegion.GetSize();

}


template< class TFixedImageType, class TMovingImageType >
void
RegistrationInterface< TFixedImageType, TMovingImageType >
::SetMovingImage( Self::MovingImagePointer movingImage )
{
	m_MovingImage = movingImage;
	m_MovingRegion = m_MovingImage->GetLargestPossibleRegion();

	m_MovingImageSize = m_MovingRegion.GetSize();
}

template< class TFixedImageType, class TMovingImageType >
void
RegistrationInterface< TFixedImageType, TMovingImageType >
::SetMetric( Self::MetricType metric )
{
	m_Metric = metric;
}

template< class TFixedImageType, class TMovingImageType >
void
RegistrationInterface< TFixedImageType, TMovingImageType >
::SetRegistrationMethod( Self::RegistrationMethodType registrationMethod )
{
	m_RegistrationMethod = registrationMethod;
}

template< class TFixedImage, class TMovingImage >
void
RegistrationInterface< TFixedImage, TMovingImage >
::SetOptimizer( Self::OptimizerType optimizer )
{
	m_Optimizer = optimizer;
}

}
