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
//rigid:
#include "itkVersorRigid3DTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkCenteredEuler3DTransform.h"

//affine:
#include "itkAffineTransform.h"
#include "itkCenteredAffineTransform.h"

//metric includes
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"

//optimizer inlcudes
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"

//interpolator includes
#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"

//resample include
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

//initializer includes
#include "itkCenteredTransformInitializer.h"

//includes for the mask creation
#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageMaskSpatialObject.h"
#include "itkOtsuThresholdImageFilter.h"

namespace isis {

template<class TFixedImageType, class TMovingImageType>
class RegistrationFactory3D: public itk::LightObject {
public:

		itkStaticConstMacro( FixedImageDimension, unsigned int, TFixedImageType::ImageDimension );
		itkStaticConstMacro( MovingImageDimension, unsigned int, TFixedImageType::ImageDimension );

	typedef RegistrationFactory3D Self;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;

	itkNewMacro( Self )
	;

	typedef TFixedImageType FixedImageType;
	typedef TMovingImageType MovingImageType;
	typedef TFixedImageType OutputImageType;
	typedef unsigned char MaskPixelType;

	//typedefs for the joint mask creation

	typedef itk::Image<MaskPixelType, FixedImageDimension> MaskImageType;

	typedef itk::BinaryThresholdImageFilter<FixedImageType, MaskImageType>
			FixedThresholdFilterType;
	typedef itk::BinaryThresholdImageFilter<MovingImageType, MaskImageType>
			MovingThresholdFilterType;
	typedef itk::MinimumMaximumImageCalculator<FixedImageType>
			FixedMinMaxCalculatorType;
	typedef itk::MinimumMaximumImageCalculator<MovingImageType>
			MovingMinMaxCalculatorType;
	typedef itk::ImageMaskSpatialObject<FixedImageDimension> MaskObjectType;

	typedef itk::OtsuThresholdImageFilter<TFixedImageType, MaskImageType>
			OtsuThresholdFilterType;

	typedef typename FixedImageType::RegionType FixedImageRegionType;
	typedef typename MovingImageType::RegionType MovingImageRegionType;

	typedef typename FixedImageType::Pointer FixedImagePointer;
	typedef typename MovingImageType::Pointer MovingImagePointer;
	typedef typename OutputImageType::Pointer OutputImagePointer;

	typedef typename FixedImageType::PixelType FixedPixelType;
	typedef typename MovingImageType::PixelType MovingPixelType;

	typedef itk::ImageRegistrationMethod<TFixedImageType, TMovingImageType>
			RegistrationMethodType;

	typedef typename RegistrationMethodType::Pointer RegistrationMethodPointer;

	//resample filter and caster
	typedef typename itk::ResampleImageFilter<MovingImageType, FixedImageType>
			ResampleFilterType;
	typedef typename itk::CastImageFilter<FixedImageType, OutputImageType>
			ImageCasterType;

	//interpolator typedefs
	typedef itk::LinearInterpolateImageFunction<MovingImageType, double>
			LinearInterpolatorType;

	typedef itk::BSplineInterpolateImageFunction<TMovingImageType, double,
			double> BSplineInterpolatorType;

	//transform typedefs
	typedef itk::SmartPointer<
			const typename RegistrationMethodType::TransformType>
			ConstTransformPointer;

	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::QuaternionRigidTransform<double> QuaternionRigidTransformType;
	typedef itk::CenteredEuler3DTransform<double> CenteredEuler3DTransformType;

	typedef itk::AffineTransform<double, FixedImageDimension>
			AffineTransformType;

	typedef itk::CenteredAffineTransform<double, FixedImageDimension>
			CenteredAffineTransformType;

	//optimizer typedefs
	typedef itk::RegularStepGradientDescentOptimizer
			RegularStepGradientDescentOptimizerType;
	typedef itk::VersorRigid3DTransformOptimizer
			VersorRigid3DTransformOptimizerType;

	//metric typedefs
	typedef itk::MattesMutualInformationImageToImageMetric<TFixedImageType,
			TMovingImageType> MattesMutualInformationMetricType;

	typedef typename itk::NormalizedCorrelationImageToImageMetric<
			TFixedImageType, TMovingImageType> NormalizedCorrelationMetricType;

	//initializer typedefs
	typedef itk::CenteredTransformInitializer<VersorRigid3DTransformType,
			FixedImageType, MovingImageType>
			VersorRigid3DTransformInitializerType;

	enum eTransformType {
		VersorRigid3DTransform,
		QuaternionRigidTransform,
		CenteredEuler3DTransform,
		AffineTransform,
		CenteredAffineTransform

	};

	enum eMetricType {
		MattesMutualInformation, NormalizedCorrelation
	};

	enum eOptimizerType {
		RegularStepGradientDescentOptimizer, VersorRigidOptimizer
	};

	enum eInterpolationType {
		LinearInterpolator, BSplineInterpolator
	};

	struct {
		unsigned int NumberOfIterations;
		unsigned int NumberOfBins;
		float PixelDensity;
		bool INITIALIZEMOMENTS;
		bool INITIALIZEGEOMETRY;
		bool PRINTRESULTS;
		bool USEOTSUTHRESHOLDING; //using an otsu threshold filter to create a mask which is designed to restrict the region given to the metric

	} UserOptions;

	void UpdateParameters(void);
	void StartRegistration(void);

	//setter methods
	void SetTransform(eTransformType);
	void SetMetric(eMetricType);
	void SetOptimizer(eOptimizerType);
	void SetInterpolator(eInterpolationType);

	void SetFixedImage(FixedImagePointer);
	void SetMovingImage(MovingImagePointer);

	//parameter-set methods
	void SetUpOptimizer(void);
	void SetUpTransform(void);
	void SetUpMetric(void);

	//getter methods
	RegistrationMethodPointer GetRegistrationObject(void);
	OutputImagePointer GetRegisteredImage(void);
	ConstTransformPointer GetTransform(void);

protected:
	void SetInitializeTransform(void);
	void PrintResults(void);
	void CheckImageSizes(void);
	void SetFixedImageMask(void);

	RegistrationFactory3D();
	virtual ~RegistrationFactory3D() {
	}

private:

	struct {
		bool VERSORRIGID;
		bool QUATERNIONRIGID;
		bool CENTEREDEULER3DTRANSFORM;
		bool AFFINE;
		bool CENTEREDAFFINE;
	} transform;

	struct Optimizer {
		bool REGULARSTEPGRADIENTDESCENT;
		bool VERSORRIGID3D;

	} optimizer;

	struct Metric {
		bool MATTESMUTUALINFORMATION;
		bool NORMALIZEDCORRELATION;
	} metric;
	struct Interpolator {
		bool LINEAR;
		bool BSPLINE;
	};

	FixedImagePointer m_FixedImage;
	MovingImagePointer m_MovingImage;
	OutputImagePointer m_OutputImage;

	typename MaskObjectType::Pointer m_MovingImageMaskObject;

	FixedImageRegionType m_FixedImageRegion;
	MovingImageRegionType m_MovingImageRegion;

	bool m_FixedImageIsBigger;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	typename MovingThresholdFilterType::Pointer m_MovingThresholdFilter;
	typename MovingMinMaxCalculatorType::Pointer m_MovingMinMaxCalculator;

	typename ResampleFilterType::Pointer m_ResampleFilter;
	typename ImageCasterType::Pointer m_ImageCaster;

	typename OtsuThresholdFilterType::Pointer m_OtsuThresholdFilter;

	//registration method
	RegistrationMethodPointer m_RegistrationObject;

	//optimizer
	RegularStepGradientDescentOptimizerType::Pointer
			m_RegularStepGradientDescentOptimizer;
	VersorRigid3DTransformOptimizerType::Pointer
			m_VersorRigid3DTransformOptimizer;

	//transform
	VersorRigid3DTransformType::Pointer m_VersorRigid3DTransform;
	QuaternionRigidTransformType::Pointer m_QuaternionRigidTransform;
	CenteredEuler3DTransformType::Pointer m_CenteredEuler3DTransform;

	typename AffineTransformType::Pointer m_AffineTransform;
	typename CenteredAffineTransformType::Pointer m_CenteredAffineTransform;

	//metric
	typename MattesMutualInformationMetricType::Pointer
			m_MattesMutualInformationMetric;

	typename NormalizedCorrelationMetricType::Pointer
			m_NormalizedCorrelationMetric;

	//interpolator
	typename LinearInterpolatorType::Pointer m_LinearInterpolator;
	typename BSplineInterpolatorType::Pointer m_BSplineInterpolator;

	//initializer
	typename VersorRigid3DTransformInitializerType::Pointer
			m_VersorRigid3DTransformInitializer;

};

}

#if ITK_TEMPLATE_TXX
# include "isisRegistrationFactory3D.txx"
#endif

#endif /* ISISREGISTRATIONFACTORY_H_ */
