/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 *****************************************************************/


#ifndef ISISREGISTRATIONFACTORY_H_
#define ISISREGISTRATIONFACTORY_H_

#include "itkImageRegistrationMethod.h"

#include "extITK/isisIterationObserver.hpp"
#include "extITK/isisScaleEstimateFilter.hpp"

//transform includes

//rigid:
#include "itkTranslationTransform.h"
#include "itkVersorRigid3DTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkCenteredEuler3DTransform.h"
#include "itkRigid3DTransform.h"

//affine:
#include "itkAffineTransform.h"
#include "itkCenteredAffineTransform.h"
#include "itkScaleSkewVersor3DTransform.h"


//non-linear
#include "itkBSplineDeformableTransform.h"

//metric includes
#include "itkMutualInformationImageToImageMetric.h" //Viola-Wells-Mutual
#include "itkMutualInformationHistogramImageToImageMetric.h"
#include "itkMattesMutualInformationImageToImageMetric.h"
#include "itkNormalizedCorrelationImageToImageMetric.h"
#include "itkMeanSquaresImageToImageMetric.h"

//optimizer inlcudes
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkLBFGSBOptimizer.h"
#include "itkAmoebaOptimizer.h"
#include "itkPowellOptimizer.h"

//interpolator includes
#include "itkLinearInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

//resample include
#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"

//includes for the mask creation
#include "itkBinaryThresholdImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageMaskSpatialObject.h"
#include "itkOtsuThresholdImageFilter.h"

//additional include stuff
#include "itkNormalizeImageFilter.h" //for ViolaWellMutualInformation. Sets mean to zero and the variance to 1
#include "itkDiscreteGaussianImageFilter.h" // low pass filtering for ViolaWellsMutualInformation to increase robustness against noise.
#include "itkCenteredTransformInitializer.h"
#include "itkLandmarkBasedTransformInitializer.h"

#include "itkPointSet.h"

namespace isis
{
namespace registration
{

template<class TFixedImageType, class TMovingImageType>
class RegistrationFactory3D : public itk::LightObject
{
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
	typedef float InternalPixelType;

	typedef double CoordinateRepType; //type for coordinates representation used by the BSplineDeformableTransform

	typedef itk::Vector<float, FixedImageDimension> VectorType;
	typedef itk::Image<VectorType, FixedImageDimension> DeformationFieldType;
	typedef typename DeformationFieldType::Pointer DeformationFieldPointer;

	//typedefs for the joint mask creation

	typedef itk::Image<MaskPixelType, FixedImageDimension> MaskImageType;
	typedef typename itk::PointSet<float, FixedImageDimension> PointSetType;

	typedef itk::BinaryThresholdImageFilter<FixedImageType, MaskImageType> FixedThresholdFilterType;
	typedef itk::BinaryThresholdImageFilter<MovingImageType, MaskImageType> MovingThresholdFilterType;
	typedef itk::MinimumMaximumImageCalculator<FixedImageType> FixedMinMaxCalculatorType;
	typedef itk::MinimumMaximumImageCalculator<MovingImageType> MovingMinMaxCalculatorType;
	typedef itk::ImageMaskSpatialObject<FixedImageDimension> MaskObjectType;

	typedef itk::OtsuThresholdImageFilter<TFixedImageType, MaskImageType> OtsuThresholdFilterType;

	typedef typename FixedImageType::RegionType FixedImageRegionType;
	typedef typename MovingImageType::RegionType MovingImageRegionType;

	typedef typename FixedImageType::Pointer FixedImagePointer;
	typedef typename MovingImageType::Pointer MovingImagePointer;
	typedef typename OutputImageType::Pointer OutputImagePointer;

	typedef typename FixedImageType::PixelType FixedPixelType;
	typedef typename MovingImageType::PixelType MovingPixelType;
	typedef itk::Image<InternalPixelType, FixedImageDimension> InternalImageType;

	typedef itk::ImageRegistrationMethod<TFixedImageType, TMovingImageType> RegistrationMethodType;

	typedef typename RegistrationMethodType::Pointer RegistrationMethodPointer;

	typedef extitk::ScaleEstimateFilter<TFixedImageType, TMovingImageType> ScaleEstimateFilterType;

	//resample filter and caster
	typedef typename itk::ResampleImageFilter<MovingImageType, FixedImageType> ResampleFilterType;
	typedef typename itk::CastImageFilter<FixedImageType, OutputImageType> ImageCasterType;

	//interpolator typedefs
	typedef itk::LinearInterpolateImageFunction<MovingImageType, double> LinearInterpolatorType;

	typedef itk::BSplineInterpolateImageFunction<TMovingImageType, double, double> BSplineInterpolatorType;

	typedef itk::NearestNeighborInterpolateImageFunction<TMovingImageType, double> NearestNeighborInterpolatorType;

	//transform typedefs
	typedef itk::TransformBase *TransformBasePointer; //not allowed to be a itk::SmartPointer because of static_cast usage
	typedef const itk::TransformBase *ConstTransformBasePointer;
	typedef itk::Transform<double, 3, 3> TransformType;

	typedef itk::TranslationTransform<double, 3> TranslationTransformType;
	typedef itk::VersorRigid3DTransform<double> VersorRigid3DTransformType;
	typedef itk::Rigid3DTransform<double> Rigid3DTransformType;
	typedef itk::QuaternionRigidTransform<double> QuaternionRigidTransformType;
	typedef itk::CenteredEuler3DTransform<double> CenteredEuler3DTransformType;

	typedef itk::AffineTransform<double, FixedImageDimension> AffineTransformType;

	typedef itk::CenteredAffineTransform<double, FixedImageDimension> CenteredAffineTransformType;
	typedef itk::ScaleSkewVersor3DTransform<double> ScaleSkewVersor3DTransformType;

	typedef itk::Transform<double, 3, 3> BulkTransformType;

	typedef typename itk::BSplineDeformableTransform<CoordinateRepType, FixedImageDimension, 3> BSplineTransformType;

	//optimizer typedefs
	typedef itk::RegularStepGradientDescentOptimizer RegularStepGradientDescentOptimizerType;
	typedef itk::VersorRigid3DTransformOptimizer VersorRigid3DTransformOptimizerType;
	typedef itk::LBFGSBOptimizer LBFGSBOptimizerType;
	typedef itk::AmoebaOptimizer AmoebaOptimizerType;
	typedef itk::PowellOptimizer PowellOptimizerType;

	//metric typedefs
	typedef itk::MattesMutualInformationImageToImageMetric<TFixedImageType, TMovingImageType>
	MattesMutualInformationMetricType;

	typedef typename itk::NormalizedCorrelationImageToImageMetric<TFixedImageType, TMovingImageType>
	NormalizedCorrelationMetricType;

	typedef typename itk::MutualInformationImageToImageMetric<TFixedImageType, TMovingImageType>
	ViolaWellsMutualInformationMetricType;

	typedef typename itk::MeanSquaresImageToImageMetric<TFixedImageType, TMovingImageType>
	MeanSquareImageToImageMetricType;

	typedef typename itk::MutualInformationHistogramImageToImageMetric<TFixedImageType, TMovingImageType>
	MutualInformationHistogramMetricType;
	//additional typedefs
	typedef typename itk::NormalizeImageFilter<TFixedImageType, TFixedImageType> FixedNormalizeImageFilterType;

	typedef typename itk::NormalizeImageFilter<TMovingImageType, TMovingImageType> MovingNormalizeImageFilterType;

	typedef typename itk::DiscreteGaussianImageFilter<TFixedImageType, TFixedImageType> DiscreteGaussianImageFitlerType;

	typedef typename itk::CenteredTransformInitializer<VersorRigid3DTransformType, TFixedImageType, TMovingImageType>
	RigidCenteredTransformInitializerType;

	typedef typename itk::CenteredTransformInitializer<AffineTransformType, TFixedImageType, TMovingImageType>
	AffineCenteredTransformInitializerType;

	typedef typename itk::LandmarkBasedTransformInitializer < VersorRigid3DTransformType, TFixedImageType,
			TMovingImageType > RigidLandmarkBasedTransformInitializerType;

	enum eTransformType {
		TranslationTransform,
		VersorRigid3DTransform,
		AffineTransform,
		CenteredAffineTransform,
		BSplineDeformableTransform,
		ScaleTransform,
		Rigid3DTransform

	};

	enum eMetricType {
		MattesMutualInformationMetric,
		ViolaWellsMutualInformationMetric,
		NormalizedCorrelationMetric,
		MeanSquareMetric,
		MutualInformationHistogramMetric
	};

	enum eOptimizerType {
		RegularStepGradientDescentOptimizer, VersorRigidOptimizer, LBFGSBOptimizer, AmoebaOptimizer, PowellOptimizer

	};

	enum eInterpolationType {
		LinearInterpolator, BSplineInterpolator, NearestNeighborInterpolator
	};

	struct {
		unsigned int NumberOfIterations;
		unsigned int NumberOfBins;
		unsigned int BSplineGridSize;
		float BSplineBound;
		unsigned int NumberOfThreads;
		unsigned int MattesMutualInitializeSeed;
		float CoarseFactor;
		float PixelDensity;
		bool PRINTRESULTS;
		bool USEOTSUTHRESHOLDING; //using an otsu threshold filter to create a mask which is designed to restrict the region given to the metric
		bool INITIALIZEMASSOFF;
		bool INITIALIZECENTEROFF;
		unsigned int SHOWITERATIONATSTEP;
		bool USEMASK;
		bool LANDMARKINITIALIZE;
	} UserOptions;

	void Reset(
		void );
	void UpdateParameters(
		void );
	void StartRegistration(
		void );

	//setter methods
	void SetTransform(
		eTransformType );
	void SetMetric(
		eMetricType );
	void SetOptimizer(
		eOptimizerType );
	void SetInterpolator(
		eInterpolationType );

	void SetFixedImage(
		FixedImagePointer );
	void SetMovingImage(
		MovingImagePointer );

	//parameter-set methods
	void SetUpOptimizer(
		void );
	void SetUpTransform(
		void );
	void SetUpMetric(
		void );

	void SetInitialTransform(
		TransformBasePointer );

	void SetMovingPointContainer(
		typename RigidLandmarkBasedTransformInitializerType::LandmarkPointContainer );
	void SetFixedPointContainer(
		typename RigidLandmarkBasedTransformInitializerType::LandmarkPointContainer );

	//getter methods
	RegistrationMethodPointer GetRegistrationObject(
		void );
	OutputImagePointer GetRegisteredImage(
		void );
	ConstTransformBasePointer GetTransform(
		void );

	DeformationFieldPointer GetTransformVectorField(
		void );

	void PrintResults(
		void );
	void CheckImageSizes(
		void );
	void SetFixedImageMask(
		typename MaskObjectType::Pointer );

	RegistrationFactory3D();
	virtual ~RegistrationFactory3D() {
	}

private:

	struct {
		bool TRANSLATION;
		bool VERSORRIGID;
		bool AFFINE;
		bool CENTEREDAFFINE;
		bool BSPLINEDEFORMABLETRANSFORM;
		bool SCALE;
		bool RIGID3D;
	} transform;

	struct Optimizer {
		bool REGULARSTEPGRADIENTDESCENT;
		bool VERSORRIGID3D;
		bool LBFGSBOPTIMIZER;
		bool AMOEBA;
		bool POWELL;

	} optimizer;

	struct Metric {
		bool MATTESMUTUALINFORMATION;
		bool NORMALIZEDCORRELATION;
		bool VIOLAWELLSMUTUALINFORMATION;
		bool MEANSQUARE;
		bool MUTUALINFORMATIONHISTOGRAM;
	} metric;

	struct Interpolator {
		bool LINEAR;
		bool BSPLINE;
		bool NEARESTNEIGHBOR;
	} interpolator;

	void SetFixedImageMask(
		void );

	DeformationFieldPointer m_DeformationField;
	FixedImagePointer m_FixedImage;
	MovingImagePointer m_MovingImage;
	OutputImagePointer m_OutputImage;

	typename MaskObjectType::Pointer m_MovingImageMaskObject;

	FixedImageRegionType m_FixedImageRegion;
	MovingImageRegionType m_MovingImageRegion;

	bool m_FixedImageIsBigger;
	bool m_InitialTransformIsSet;

	unsigned int m_NumberOfParameters;

	isis::extitk::IterationObserver::Pointer m_observer;

	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	typename MovingThresholdFilterType::Pointer m_MovingThresholdFilter;
	typename MovingMinMaxCalculatorType::Pointer m_MovingMinMaxCalculator;

	typename ResampleFilterType::Pointer m_ResampleFilter;
	typename ImageCasterType::Pointer m_ImageCaster;

	typename OtsuThresholdFilterType::Pointer m_OtsuThresholdFilter;

	typename RigidCenteredTransformInitializerType::Pointer m_RigidInitializer;
	typename AffineCenteredTransformInitializerType::Pointer m_AffineInitializer;

	typename RigidLandmarkBasedTransformInitializerType::Pointer m_RigidLandmarkInitializer;
	typename RigidLandmarkBasedTransformInitializerType::LandmarkPointContainer m_MovingPointContainer;
	typename RigidLandmarkBasedTransformInitializerType::LandmarkPointContainer m_FixedPointContainer;

	//registration method
	RegistrationMethodPointer m_RegistrationObject;

	//optimizer
	RegularStepGradientDescentOptimizerType::Pointer m_RegularStepGradientDescentOptimizer;
	VersorRigid3DTransformOptimizerType::Pointer m_VersorRigid3DTransformOptimizer;
	LBFGSBOptimizerType::Pointer m_LBFGSBOptimizer;
	AmoebaOptimizerType::Pointer m_AmoebaOptimizer;
	PowellOptimizerType::Pointer m_PowellOptimizer;

	//transform
	TranslationTransformType::Pointer m_TranslationTransform;
	VersorRigid3DTransformType::Pointer m_VersorRigid3DTransform;
	Rigid3DTransformType::Pointer m_Rigid3DTransform;
	QuaternionRigidTransformType::Pointer m_QuaternionRigidTransform;
	CenteredEuler3DTransformType::Pointer m_CenteredEuler3DTransform;

	typename AffineTransformType::Pointer m_AffineTransform;
	typename ScaleSkewVersor3DTransformType::Pointer m_ScaleSkewTransform;
	typename CenteredAffineTransformType::Pointer m_CenteredAffineTransform;

	typename BSplineTransformType::Pointer m_BSplineTransform;

	BulkTransformType::Pointer m_BulkTransform;

	//metric
	typename MattesMutualInformationMetricType::Pointer m_MattesMutualInformationMetric;
	typename NormalizedCorrelationMetricType::Pointer m_NormalizedCorrelationMetric;
	typename ViolaWellsMutualInformationMetricType::Pointer m_ViolaWellsMutualInformationMetric;
	typename MeanSquareImageToImageMetricType::Pointer m_MeanSquareMetric;
	typename MutualInformationHistogramMetricType::Pointer m_MutualInformationHistogramMetric;

	//interpolator
	typename LinearInterpolatorType::Pointer m_LinearInterpolator;
	typename BSplineInterpolatorType::Pointer m_BSplineInterpolator;
	typename NearestNeighborInterpolatorType::Pointer m_NearestNeighborInterpolator;

	//additional declarations
	typename FixedNormalizeImageFilterType::Pointer m_FixedNormalizeImageFilter;
	typename MovingNormalizeImageFilterType::Pointer m_MovingNormalizeImageFilter;

	typename DiscreteGaussianImageFitlerType::Pointer m_FixedGaussianFilter;
	typename DiscreteGaussianImageFitlerType::Pointer m_MovingGaussianFilter;

	ScaleEstimateFilterType *m_ScaleEstimateFilter;
	typename ScaleEstimateFilterType::ScaleType m_EstimatedScaling;

};

} //end namespace Registration

} //end namespace isis

#if ITK_TEMPLATE_TXX
# include "isisRegistrationFactory3D.txx"
#endif

#endif /* ISISREGISTRATIONFACTORY_H_ */
