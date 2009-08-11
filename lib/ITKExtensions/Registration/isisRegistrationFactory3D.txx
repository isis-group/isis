/*
 * isisRegistrationFactory.txx
 *
 *  Created on: Jul 13, 2009
 *      Author: tuerke
 */

#include "isisRegistrationFactory3D.h"

namespace isis {


template< class TFixedImageType, class TMovingImageType >
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::RegistrationFactory3D()
{

	m_RegistrationObject = RegistrationMethodType::New();

	//boolean settings
	optimizer.REGULARSTEPGRADIENTDESCENT = false;
	optimizer.VERSORRIGID3D = false;


	transform.VERSORRIGID = false;
	transform.QUATERNIONRIGID = false;
	metric.MATTESMUTUALINFORMATIONMETRIC = false;

	m_FixedImageIsBigger = false;

	UserOptions.INITIALIZEGEOMETRY = false;
	UserOptions.INITIALIZEMOMENTS = false;
	UserOptions.METRICUSEALLPIXELS = false;
	UserOptions.PRINTRESULTS = false;
	UserOptions.NumberOfIterations = 200;


}



template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetFixedImage( FixedImagePointer fixedImage )
{
	m_FixedImage = fixedImage;
	m_RegistrationObject->SetFixedImage( m_FixedImage );
	m_FixedImageRegion = m_FixedImage->GetBufferedRegion();
	m_RegistrationObject->SetFixedImageRegion( m_FixedImageRegion );
}



template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetMovingImage( MovingImagePointer movingImage )
{
	m_MovingImage = movingImage;
	m_RegistrationObject->SetMovingImage( m_MovingImage );
	m_MovingImageRegion = m_MovingImage->GetBufferedRegion();

}




template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetMetric( eMetricType e_metric )
{
	switch (e_metric)
	{
	case MattesMutualInformation:
		metric.MATTESMUTUALINFORMATIONMETRIC = true;
		m_MattesMutualInformationMetric = MattesMutualInformationMetricType::New();
		m_RegistrationObject->SetMetric( m_MattesMutualInformationMetric );

		break;
	}
}





template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetInterpolator( eInterpolationType e_interpolator )
{
	switch (e_interpolator)
	{
	case Linear:
		m_LinearInterpolator = LinearInterpolatorType::New();
		m_RegistrationObject->SetInterpolator( m_LinearInterpolator );

		break;


	}

}


template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetTransform( eTransformType e_transform )
{
	switch (e_transform)
	{
	case VersorRigid3DTransform:
		transform.VERSORRIGID = true;
		m_VersorRigid3DTransform = VersorRigid3DTransformType::New();
		m_RegistrationObject->SetTransform( m_VersorRigid3DTransform );
		break;
	case QuaternionRigidTransform:
		transform.QUATERNIONRIGID = true;
		m_QuaternionRigidTransform = QuaternionRigidTransformType::New();
		m_RegistrationObject->SetTransform( m_QuaternionRigidTransform );
		break;



	}
}

template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetOptimizer( eOptimizerType e_optimizer )
{
	switch( e_optimizer )
	{
	case RegularStepGradientDescentOptimizer:
		optimizer.REGULARSTEPGRADIENTDESCENT = true;
		m_RegularStepGradientDescentOptimizer =
				RegularStepGradientDescentOptimizerType::New();
		m_RegistrationObject->SetOptimizer( m_RegularStepGradientDescentOptimizer );
	case VersorRigidOptimizer:
		optimizer.VERSORRIGID3D = true;
		m_VersorRigid3DTransformOptimizer =
				VersorRigid3DTransformOptimizerType::New();
		m_RegistrationObject->SetOptimizer( m_VersorRigid3DTransformOptimizer );
		break;


	}
}





//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++parameter setting methods++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::UpdateParameters()
{

	//optimizer parameters:
	this->SetUpOptimizer();

	//transform parameters:
	this->SetUpTransform();

	//metric parameters;
	this->SetUpMetric();




}









template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetUpOptimizer()
{
	if( optimizer.REGULARSTEPGRADIENTDESCENT )
	{
		//setting up the regular step gradient descent optimizer...
		RegularStepGradientDescentOptimizerType::ScalesType optimizerScale
										 ( m_RegistrationObject->GetTransform()->GetNumberOfParameters() );
		unsigned int NumberOfParameters = m_RegistrationObject->GetTransform()->GetNumberOfParameters();
		if( transform.VERSORRIGID or transform.QUATERNIONRIGID )
		{
		//...for the rigid transform
			//number of parameters are dependent on the dimension of the images (2D: 4 parameter, 3D: 6 parameters)
			for( unsigned int i = 0; i < NumberOfParameters; i++ )
			{
				optimizerScale[i] = 1.0/1000.0;
			}
			m_RegularStepGradientDescentOptimizer->SetMaximumStepLength( 0.1 );
			m_RegularStepGradientDescentOptimizer->SetMinimumStepLength( 0.0001 );
			m_RegularStepGradientDescentOptimizer->SetScales( optimizerScale );
			m_RegularStepGradientDescentOptimizer->SetNumberOfIterations( UserOptions.NumberOfIterations );
		}
	}
	if( optimizer.VERSORRIGID3D )
	{
		VersorRigid3DTransformOptimizerType::ScalesType optimizerScale
										( m_RegistrationObject->GetTransform()->GetNumberOfParameters() );
		unsigned  int NumberOfParameters = m_RegistrationObject->GetTransform()->GetNumberOfParameters();
		if( transform.VERSORRIGID or transform.QUATERNIONRIGID )
		{
			optimizerScale[0] = 1.0;
			for( unsigned int i = 1; i < NumberOfParameters; i++ )
			{
				optimizerScale[i] = 1.0/1000.0;
			}
			m_VersorRigid3DTransformOptimizer->SetMaximumStepLength( 0.1 );
			m_VersorRigid3DTransformOptimizer->SetMinimumStepLength( 0.0001 );
			m_VersorRigid3DTransformOptimizer->SetScales( optimizerScale );
			m_VersorRigid3DTransformOptimizer->SetNumberOfIterations( UserOptions.NumberOfIterations );
		}

	}


}

template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetUpTransform()
{
	if( transform.VERSORRIGID )
	{
		//setting up the rigid transform



	}
}



template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetUpMetric()
{
	if( metric.MATTESMUTUALINFORMATIONMETRIC )
	{
		//setting up the mattes mutual information metric
		m_MattesMutualInformationMetric->SetFixedImage( m_FixedImage );
		m_MattesMutualInformationMetric->SetMovingImage( m_MovingImage );
		m_MattesMutualInformationMetric->SetFixedImageRegion( m_FixedImageRegion );
		if( UserOptions.METRICUSEALLPIXELS )
		{
			m_MattesMutualInformationMetric->UseAllPixelsOn();
		}
		else
		{
			m_MattesMutualInformationMetric->SetNumberOfSpatialSamples(
												m_FixedImageRegion.GetNumberOfPixels() * 0.01 );

		}
		m_MattesMutualInformationMetric->SetNumberOfHistogramBins( 30 );

	}

}




template< class TFixedImageType, class TMovingImageType >
typename RegistrationFactory3D< TFixedImageType, TMovingImageType >::OutputImagePointer
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::GetRegisteredImage( void )
{
	m_ResampleFilter = ResampleFilterType::New();
	m_ImageCaster = ImageCasterType::New();

	m_ResampleFilter->SetInput( m_MovingImage );
	m_ResampleFilter->SetTransform( m_RegistrationObject->GetOutput()->Get() );
	m_ResampleFilter->SetOutputOrigin( m_FixedImage->GetOrigin() );
	m_ResampleFilter->SetSize( m_FixedImage->GetLargestPossibleRegion().GetSize() );
	m_ResampleFilter->SetOutputSpacing( m_FixedImage->GetSpacing() );
	m_ResampleFilter->SetOutputDirection( m_FixedImage->GetDirection() );
	m_ResampleFilter->SetDefaultPixelValue( 0 );
	m_ImageCaster->SetInput( m_ResampleFilter->GetOutput() );
	m_OutputImage = m_ImageCaster->GetOutput();
	m_ImageCaster->Update();
	return m_OutputImage;
}

template< class TFixedImageType, class TMovingImageType >
typename RegistrationFactory3D< TFixedImageType, TMovingImageType >::ConstTransformPointer
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::GetTransform( void )
{

	return m_RegistrationObject->GetOutput()->Get();
}

template< class TFixedImageType, class TMovingImageType >
typename RegistrationFactory3D< TFixedImageType, TMovingImageType >::RegistrationMethodPointer
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::GetRegistrationObject( void )
{
	this->UpdateParameters();
	return m_RegistrationObject;

}

template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetInitializeTransform( void )
{

	if( transform.VERSORRIGID )
	{
		if( UserOptions.INITIALIZEGEOMETRY or UserOptions.INITIALIZEMOMENTS )
		//the user has selected either the geometry initializer, which alignes the geometrical centers
		//of the images or the moment initializer, which alignes the center of mass of the images
		{
			m_VersorRigid3DTransformInitializer = VersorRigid3DTransformInitializerType::New();
			m_VersorRigid3DTransformInitializer->SetTransform( m_VersorRigid3DTransform );
			m_VersorRigid3DTransformInitializer->SetFixedImage( m_FixedImage );
			m_VersorRigid3DTransformInitializer->SetMovingImage( m_MovingImage );
			if( UserOptions.INITIALIZEGEOMETRY )
			{
				m_VersorRigid3DTransformInitializer->GeometryOn();
			}
			if( UserOptions.INITIALIZEMOMENTS )
			{
				m_VersorRigid3DTransformInitializer->MomentsOn();
			}
			m_VersorRigid3DTransformInitializer->InitializeTransform();

			m_RegistrationObject->SetInitialTransformParameters( m_VersorRigid3DTransform->GetParameters() );
		}
		else //the user has not set a specific initializer
		{
			m_RegistrationObject->SetInitialTransformParameters( m_VersorRigid3DTransform->GetParameters() );
		}

	}
	if( transform.QUATERNIONRIGID )
	{
		m_RegistrationObject->SetInitialTransformParameters( m_QuaternionRigidTransform->GetParameters() );
	}

}



template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::CheckImageSizes( void )
{
	for( int i=0; i < FixedImageDimension; i++ )
	{
		if( m_FixedImageRegion.GetSize()[i] > m_MovingImageRegion.GetSize()[i] )
		{
			m_FixedImageIsBigger = true;
		}
	}
	if( m_FixedImageIsBigger )
	{
		m_FixedImageMask = MaskImageType::New();
		m_MovingImageMask = MaskImageType::New();
		m_JointImageMask = MaskObjectType::New();
		m_AndFilter = AndFilterType::New();
		m_FixedThresholdFilter = FixedThresholdFilterType::New();
		m_MovingThresholdFilter = MovingThresholdFilterType::New();
		m_FixedMinMaxCalculator = FixedMinMaxCalculatorType::New();
		m_MovingMinMaxCalculator = MovingMinMaxCalculatorType::New();

		m_FixedMinMaxCalculator->SetImage( m_FixedImage );
		m_FixedMinMaxCalculator->Compute();
		m_MovingMinMaxCalculator->SetImage( m_MovingImage );
		m_MovingMinMaxCalculator->Compute();

		m_FixedThresholdFilter->SetInput( m_FixedImage );
		m_FixedThresholdFilter->SetOutsideValue( 0 );
		m_FixedThresholdFilter->SetInsideValue( 255 );
		m_FixedThresholdFilter->SetUpperThreshold( m_FixedMinMaxCalculator->GetMaximum() );
		m_FixedThresholdFilter->SetLowerThreshold( m_FixedMinMaxCalculator->GetMinimum() );
		m_FixedThresholdFilter->Update();

		m_MovingThresholdFilter->SetInput( m_MovingImage );
		m_MovingThresholdFilter->SetOutsideValue( 0 );
		m_MovingThresholdFilter->SetInsideValue( 255 );
		m_MovingThresholdFilter->SetUpperThreshold( m_MovingMinMaxCalculator->GetMaximum() );
		m_MovingThresholdFilter->SetLowerThreshold( m_MovingMinMaxCalculator->GetMinimum() );
		m_MovingThresholdFilter->UpdateLargestPossibleRegion();
		m_MovingThresholdFilter->Update();

		m_AndFilter->SetInput1( m_MovingThresholdFilter->GetOutput() );
		m_AndFilter->SetInput2( m_FixedThresholdFilter->GetOutput() );

		m_AndFilter->Update();

		m_JointImageMask->SetImage( m_AndFilter->GetOutput() );
		m_JointImageMask->Update();
		this->SetFixedImageMask();


	}


}

template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::SetFixedImageMask( void )
{
	if( metric.MATTESMUTUALINFORMATIONMETRIC )
	{
		m_MattesMutualInformationMetric->SetFixedImageMask( m_JointImageMask );
	}
}



template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::PrintResults( void )
{
	std::cout << "Results of registration: " << std::endl << std::endl;
	if( transform.VERSORRIGID )
	{
		std::cout << "Versor x: " << m_RegistrationObject->GetLastTransformParameters()[0] << std::endl;
		std::cout << "Versor y: " << m_RegistrationObject->GetLastTransformParameters()[1] << std::endl;
		std::cout << "Versor z: " << m_RegistrationObject->GetLastTransformParameters()[2] << std::endl;
		std::cout << "Translation x: " << m_RegistrationObject->GetLastTransformParameters()[3] << std::endl;
		std::cout << "Translation y: " << m_RegistrationObject->GetLastTransformParameters()[4] << std::endl;
		std::cout << "Translation z: " << m_RegistrationObject->GetLastTransformParameters()[5] << std::endl;
	}
	if( optimizer.REGULARSTEPGRADIENTDESCENT )
	{
		std::cout << "Iterations: " << m_RegularStepGradientDescentOptimizer->GetCurrentIteration() << std::endl;
		std::cout << "Metric value: " << m_RegularStepGradientDescentOptimizer->GetValue() << std::endl;

	}

}


template< class TFixedImageType, class TMovingImageType >
void
RegistrationFactory3D< TFixedImageType, TMovingImageType >
::StartRegistration( void )
{
	//set all parameters to make sure all user changes are noticed
	this->UpdateParameters();
	//check the image sizes and creat a joint image mask if the fixed image is bigger than the moving image
	//to avoid a itk sample error caused by a lack of spatial samples used by the metric
	this->CheckImageSizes();
	//set up the initial transform parameters
	this->SetInitializeTransform();


	try
	{
		m_RegistrationObject->Update();
	}
	catch ( itk::ExceptionObject & err )
	{
		std::cerr << "isRegistrationFactory3D: Exception caught: " << std::endl
	    << err << std::endl;
	}
	if( UserOptions.PRINTRESULTS )
	{
		this->PrintResults();
	}

}




} //end namespace isis
