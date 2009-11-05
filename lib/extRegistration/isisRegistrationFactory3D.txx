/*
 * isisRegistrationFactory.txx
 *
 *  Created on: Jul 13, 2009
 *      Author: tuerke
 */

#include "isisRegistrationFactory3D.h"

namespace isis {
namespace registration {

template<class TFixedImageType, class TMovingImageType>
RegistrationFactory3D<TFixedImageType, TMovingImageType>::RegistrationFactory3D() {

    m_RegistrationObject = RegistrationMethodType::New();

    this->Reset();

}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::Reset(
    void) {

    //boolean settings
    optimizer.REGULARSTEPGRADIENTDESCENT = false;
    optimizer.VERSORRIGID3D = false;
    optimizer.LBFGSBOPTIMIZER = false;
    optimizer.AMOEBA = false;
    optimizer.LEVENBERGMARQUARDT = false;

    transform.VERSORRIGID = false;
    transform.QUATERNIONRIGID = false;
    transform.CENTEREDEULER3DTRANSFORM = false;
    transform.AFFINE = false;
    transform.CENTEREDAFFINE = false;
    transform.BSPLINEDEFORMABLETRANSFORM = false;

    metric.MATTESMUTUALINFORMATION = false;
    metric.NORMALIZEDCORRELATION = false;
    metric.VIOLAWELLSMUTUALINFORMATION = false;
    metric.MEANSQUARE = false;
    metric.MUTUALINFORMATIONHISTOGRAM = false;

    interpolator.BSPLINE = false;
    interpolator.LINEAR = false;
    interpolator.NEARESTNEIGHBOR = false;

    m_FixedImageIsBigger = false;

    UserOptions.PRINTRESULTS = false;
    UserOptions.NumberOfIterations = 1000;
    UserOptions.NumberOfBins = 50;
    UserOptions.PixelDensity = 0.1;
    UserOptions.USEOTSUTHRESHOLDING = false;
    UserOptions.BSplineGridSize = 5;
    UserOptions.INITIALIZEOFF = false;
    UserOptions.NumberOfThreads = 1;
    UserOptions.MattesMutualInitializeSeed = 1;
    UserOptions.SHOWITERATIONSTATUS = false;

    m_NumberOfParameters = 0;
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetFixedImage(
    FixedImagePointer fixedImage) {
    m_FixedImage = fixedImage;
    m_RegistrationObject->SetFixedImage(m_FixedImage);
    m_FixedImageRegion = m_FixedImage->GetLargestPossibleRegion();
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetMovingImage(
    MovingImagePointer movingImage) {
    m_MovingImage = movingImage;
    m_RegistrationObject->SetMovingImage(m_MovingImage);
    m_MovingImageRegion = m_MovingImage->GetLargestPossibleRegion();

}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetMetric(
    eMetricType e_metric) {
    switch (e_metric) {
    case MattesMutualInformationMetric:
        metric.MATTESMUTUALINFORMATION = true;
        m_MattesMutualInformationMetric = MattesMutualInformationMetricType::New();
        m_RegistrationObject->SetMetric(m_MattesMutualInformationMetric);
        break;

    case ViolaWellsMutualInformationMetric:
        metric.VIOLAWELLSMUTUALINFORMATION = true;
        m_ViolaWellsMutualInformationMetric = ViolaWellsMutualInformationMetricType::New();
        m_RegistrationObject->SetMetric(m_ViolaWellsMutualInformationMetric);
        break;
    case MutualInformationHistogramMetric:
        metric.MUTUALINFORMATIONHISTOGRAM = true;
        m_MutualInformationHistogramMetric = MutualInformationHistogramMetricType::New();
        m_RegistrationObject->SetMetric(m_MutualInformationHistogramMetric);
        break;

    case NormalizedCorrelationMetric:
        metric.NORMALIZEDCORRELATION = true;
        m_NormalizedCorrelationMetric = NormalizedCorrelationMetricType::New();
        m_RegistrationObject->SetMetric(m_NormalizedCorrelationMetric);
        break;
    case MeanSquareMetric:
        metric.MEANSQUARE = true;
        m_MeanSquareMetric = MeanSquareImageToImageMetricType::New();
        m_RegistrationObject->SetMetric(m_MeanSquareMetric);
        break;
    }
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetInterpolator(
    eInterpolationType e_interpolator) {
    switch (e_interpolator) {
    case LinearInterpolator:
        interpolator.LINEAR = true;
        m_LinearInterpolator = LinearInterpolatorType::New();
        m_RegistrationObject->SetInterpolator(m_LinearInterpolator);
        break;

    case BSplineInterpolator:
        interpolator.BSPLINE = true;
        m_BSplineInterpolator = BSplineInterpolatorType::New();
        m_RegistrationObject->SetInterpolator(m_BSplineInterpolator);
        break;

    case NearestNeighborInterpolator:
        interpolator.NEARESTNEIGHBOR = true;
        m_NearestNeighborInterpolator = NearestNeighborInterpolatorType::New();
        m_RegistrationObject->SetInterpolator(m_NearestNeighborInterpolator);
        break;

    }
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetTransform(
    eTransformType e_transform) {
    switch (e_transform) {
    case VersorRigid3DTransform:
        transform.VERSORRIGID = true;
        m_VersorRigid3DTransform = VersorRigid3DTransformType::New();
        m_RegistrationObject->SetTransform(m_VersorRigid3DTransform);
        break;

    case QuaternionRigidTransform:
        transform.QUATERNIONRIGID = true;
        m_QuaternionRigidTransform = QuaternionRigidTransformType::New();
        m_RegistrationObject->SetTransform(m_QuaternionRigidTransform);
        break;

    case CenteredEuler3DTransform:
        transform.CENTEREDEULER3DTRANSFORM = true;
        m_CenteredEuler3DTransform = CenteredEuler3DTransformType::New();
        m_RegistrationObject->SetTransform(m_CenteredEuler3DTransform);
        break;

    case AffineTransform:
        transform.AFFINE = true;
        m_AffineTransform = AffineTransformType::New();
        m_RegistrationObject->SetTransform(m_AffineTransform);
        break;

    case CenteredAffineTransform:
        transform.CENTEREDAFFINE = true;
        m_CenteredAffineTransform = CenteredAffineTransformType::New();
        m_RegistrationObject->SetTransform(m_CenteredAffineTransform);
        break;

    case BSplineDeformableTransform:
        transform.BSPLINEDEFORMABLETRANSFORM = true;
        m_BSplineTransform = BSplineTransformType::New();
        m_RegistrationObject->SetTransform(m_BSplineTransform);
        break;

    }
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetOptimizer(
    eOptimizerType e_optimizer) {
    switch (e_optimizer) {
    case RegularStepGradientDescentOptimizer:
        optimizer.REGULARSTEPGRADIENTDESCENT = true;
        m_RegularStepGradientDescentOptimizer = RegularStepGradientDescentOptimizerType::New();
        m_RegistrationObject->SetOptimizer(m_RegularStepGradientDescentOptimizer);
        break;
    case VersorRigidOptimizer:
        optimizer.VERSORRIGID3D = true;
        m_VersorRigid3DTransformOptimizer = VersorRigid3DTransformOptimizerType::New();
        m_RegistrationObject->SetOptimizer(m_VersorRigid3DTransformOptimizer);
        break;
    case LBFGSBOptimizer:
        optimizer.LBFGSBOPTIMIZER = true;
        m_LBFGSBOptimizer = LBFGSBOptimizerType::New();
        m_RegistrationObject->SetOptimizer(m_LBFGSBOptimizer);
        break;
    case AmoebaOptimizer:
        optimizer.AMOEBA = true;
        m_AmoebaOptimizer = AmoebaOptimizerType::New();
        m_RegistrationObject->SetOptimizer(m_AmoebaOptimizer);
        break;

    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++++++++++++++++++++parameter setting methods++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::UpdateParameters() {

    //transform parameters:
    this->SetUpTransform();

    //optimizer parameters:
    this->SetUpOptimizer();
    //metric parameters;
    this->SetUpMetric();
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetUpOptimizer() {

    if (optimizer.REGULARSTEPGRADIENTDESCENT) {
        //setting up the regular step gradient descent optimizer...
        RegularStepGradientDescentOptimizerType::ScalesType optimizerScaleRegularStepGradient(m_NumberOfParameters);

        if (transform.VERSORRIGID or transform.QUATERNIONRIGID or transform.CENTEREDEULER3DTRANSFORM
                or transform.CENTEREDAFFINE or transform.AFFINE or transform.BSPLINEDEFORMABLETRANSFORM) {
            //...for the rigid transform
            //number of parameters are dependent on the dimension of the images (2D: 4 parameter, 3D: 6 parameters)

            optimizerScaleRegularStepGradient[0] = 1.0;
            optimizerScaleRegularStepGradient[1] = 1.0;
            optimizerScaleRegularStepGradient[2] = 1.0;
            for (unsigned int i = 3; i < m_NumberOfParameters; i++) {
                optimizerScaleRegularStepGradient[i] = 1.0 / 1000.0;
            }
            m_RegularStepGradientDescentOptimizer->SetMaximumStepLength(0.1);
            m_RegularStepGradientDescentOptimizer->SetMinimumStepLength(0.0001);
            m_RegularStepGradientDescentOptimizer->SetScales(optimizerScaleRegularStepGradient);
            m_RegularStepGradientDescentOptimizer->SetNumberOfIterations(UserOptions.NumberOfIterations);
            m_RegularStepGradientDescentOptimizer->SetRelaxationFactor(0.8);
            m_RegularStepGradientDescentOptimizer->SetMinimize(true);
        }

        if (metric.VIOLAWELLSMUTUALINFORMATION or metric.MUTUALINFORMATIONHISTOGRAM) {
            m_RegularStepGradientDescentOptimizer->MaximizeOn();
        }

    }
    if (optimizer.VERSORRIGID3D) {
        VersorRigid3DTransformOptimizerType::ScalesType optimizerScaleVersorRigid3D(m_NumberOfParameters);

        if (transform.VERSORRIGID or transform.QUATERNIONRIGID or transform.CENTEREDEULER3DTRANSFORM) {

            optimizerScaleVersorRigid3D[0] = 1.0;
            optimizerScaleVersorRigid3D[1] = 1.0;
            optimizerScaleVersorRigid3D[2] = 1.0;
            for (unsigned int i = 3; i < m_NumberOfParameters; i++) {
                optimizerScaleVersorRigid3D[i] = 1.0 / 1000.0;
            }
            m_VersorRigid3DTransformOptimizer->SetMaximumStepLength(0.1);
            m_VersorRigid3DTransformOptimizer->SetMinimumStepLength(0.0001);
            m_VersorRigid3DTransformOptimizer->SetScales(optimizerScaleVersorRigid3D);
            m_VersorRigid3DTransformOptimizer->SetNumberOfIterations(UserOptions.NumberOfIterations);
            m_VersorRigid3DTransformOptimizer->SetRelaxationFactor(0.8);
            m_VersorRigid3DTransformOptimizer->SetMinimize(true);

        }

    }

    if (optimizer.LBFGSBOPTIMIZER) {

        LBFGSBOptimizerType::BoundSelectionType boundSelect(m_NumberOfParameters);
        LBFGSBOptimizerType::BoundValueType lowerBound(m_NumberOfParameters);
        LBFGSBOptimizerType::BoundValueType upperBound(m_NumberOfParameters);

        boundSelect.Fill(0);
        lowerBound.Fill(0.0);
        upperBound.Fill(0.0);

        m_LBFGSBOptimizer->SetBoundSelection(boundSelect);
        m_LBFGSBOptimizer->SetLowerBound(lowerBound);
        m_LBFGSBOptimizer->SetUpperBound(upperBound);

        m_LBFGSBOptimizer->SetCostFunctionConvergenceFactor(1.e8);
        m_LBFGSBOptimizer->SetProjectedGradientTolerance(1e-9);
        m_LBFGSBOptimizer->SetMaximumNumberOfIterations(UserOptions.NumberOfIterations);
        m_LBFGSBOptimizer->SetMaximumNumberOfEvaluations(30);
        m_LBFGSBOptimizer->SetMaximumNumberOfCorrections(12);

    }
    if (optimizer.AMOEBA) {
        AmoebaOptimizerType::ParametersType simplexDelta(m_NumberOfParameters);
        //simplexDelta.Fill(5.0);
        m_AmoebaOptimizer->AutomaticInitialSimplexOn();
        m_AmoebaOptimizer->SetInitialSimplexDelta(simplexDelta);
        m_AmoebaOptimizer->SetMaximumNumberOfIterations(UserOptions.NumberOfIterations);
        m_AmoebaOptimizer->SetParametersConvergenceTolerance(1e-10);
        m_AmoebaOptimizer->SetFunctionConvergenceTolerance(1e-10);

        if (metric.VIOLAWELLSMUTUALINFORMATION or metric.MUTUALINFORMATIONHISTOGRAM) {
            m_AmoebaOptimizer->MaximizeOn();
        }
    }

    if (optimizer.LEVENBERGMARQUARDT) {
        LevenbergMarquardtOptimizerType::ScalesType optimizerScaleLevenbergMarquardt(m_NumberOfParameters);
        for (unsigned int i = 0; i < m_NumberOfParameters; i++) {
            optimizerScaleLevenbergMarquardt[i] = 1.0 / 1000.0;
        }
        m_LevenbergMarquardtOptimizer->SetScales(optimizerScaleLevenbergMarquardt);

    }

}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetUpTransform() {

    //initialize transform
    if (!UserOptions.INITIALIZEOFF) {

        if (transform.VERSORRIGID) {
		std::cout << "dhsjdhfsk" << std::endl;
            m_RigidInitializer = RigidCenteredTransformInitializerType::New();
            m_RigidInitializer->SetTransform(m_VersorRigid3DTransform);
            m_RigidInitializer->SetFixedImage(m_FixedImage);
            m_RigidInitializer->SetMovingImage(m_MovingImage);
            m_RigidInitializer->MomentsOn();

            m_RigidInitializer->InitializeTransform();
        }
        if (transform.AFFINE) {
            m_AffineInitializer = AffineCenteredTransformInitializerType::New();
            m_AffineInitializer->SetTransform(m_AffineTransform);
            m_AffineInitializer->SetFixedImage(m_FixedImage);
            m_AffineInitializer->SetMovingImage(m_MovingImage);
            m_AffineInitializer->GeometryOn();
            m_AffineInitializer->InitializeTransform();
        }
    }

    if (transform.BSPLINEDEFORMABLETRANSFORM) {

        typedef typename BSplineTransformType::RegionType BSplineRegionType;
        typedef typename BSplineTransformType::SpacingType BSplineSpacingType;
        typedef typename BSplineTransformType::OriginType BSplineOriginType;
        typedef typename BSplineTransformType::DirectionType BSplineDirectionType;

        BSplineRegionType bsplineRegion;
        typename BSplineRegionType::SizeType gridSizeOnImage;
        typename BSplineRegionType::SizeType gridBorderSize;
        typename BSplineRegionType::SizeType totalGridSize;

        gridSizeOnImage.Fill(UserOptions.BSplineGridSize);
        gridBorderSize.Fill(3); //Border for spline order = 3 (1 lower, 2 upper)
        totalGridSize = gridSizeOnImage + gridBorderSize;

        bsplineRegion.SetSize(totalGridSize);
        BSplineSpacingType bsplineSpacing = m_FixedImage->GetSpacing();

        BSplineOriginType bsplineOrigin = m_FixedImage->GetOrigin();

        typename FixedImageType::SizeType fixedImageSize = m_FixedImage->GetBufferedRegion().GetSize();

        for (unsigned int r = 0; r < FixedImageDimension; r++) {
            bsplineSpacing[r] *= static_cast<double> (fixedImageSize[r] - 1) / static_cast<double> (gridSizeOnImage[r]
                                 - 1);
        }

        BSplineDirectionType bsplineDirection = m_FixedImage->GetDirection();
        BSplineSpacingType gridOriginOffset = bsplineDirection * bsplineSpacing;

        bsplineOrigin = bsplineOrigin - gridOriginOffset;

        m_BSplineTransform->SetGridSpacing(bsplineSpacing);
        m_BSplineTransform->SetGridOrigin(bsplineOrigin);
        m_BSplineTransform->SetGridRegion(bsplineRegion);
        m_BSplineTransform->SetGridDirection(bsplineDirection);

        typedef typename BSplineTransformType::ParametersType BSplineParametersType;

        m_NumberOfParameters = m_BSplineTransform->GetNumberOfParameters();

        BSplineParametersType bsplineParameters(m_NumberOfParameters);
        bsplineParameters.Fill(0.0);

        m_BSplineTransform->SetParameters(bsplineParameters);
        m_RegistrationObject->SetInitialTransformParameters(m_BSplineTransform->GetParameters());

    }

    if (transform.AFFINE) {
        m_NumberOfParameters = m_AffineTransform->GetNumberOfParameters();
        m_RegistrationObject->SetInitialTransformParameters(m_AffineTransform->GetParameters());
    }

    if (transform.CENTEREDAFFINE) {
        m_NumberOfParameters = m_CenteredAffineTransform->GetNumberOfParameters();
        m_RegistrationObject->SetInitialTransformParameters(m_CenteredAffineTransform->GetParameters());
    }
    if (transform.VERSORRIGID) {
        m_NumberOfParameters = m_VersorRigid3DTransform->GetNumberOfParameters();
        m_RegistrationObject->SetInitialTransformParameters(m_VersorRigid3DTransform->GetParameters());
    }
    if (transform.CENTEREDEULER3DTRANSFORM) {
        m_NumberOfParameters = m_CenteredEuler3DTransform->GetNumberOfParameters();
        m_RegistrationObject->SetInitialTransformParameters(m_CenteredEuler3DTransform->GetParameters());
    }
    if (transform.QUATERNIONRIGID) {
        m_NumberOfParameters = m_QuaternionRigidTransform->GetNumberOfParameters();
        m_RegistrationObject->SetInitialTransformParameters(m_QuaternionRigidTransform->GetParameters());
    }
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetUpMetric() {
    if (metric.MATTESMUTUALINFORMATION) {
        //setting up the mattes mutual information metric
        m_MattesMutualInformationMetric->SetFixedImage(m_FixedImage);
        m_MattesMutualInformationMetric->SetMovingImage(m_MovingImage);
        m_MattesMutualInformationMetric->SetFixedImageRegion(m_FixedImageRegion);
        m_MattesMutualInformationMetric->SetNumberOfSpatialSamples(m_FixedImageRegion.GetNumberOfPixels()
                * UserOptions.PixelDensity);

        m_MattesMutualInformationMetric->SetNumberOfHistogramBins(UserOptions.NumberOfBins);
        m_MattesMutualInformationMetric->ReinitializeSeed(UserOptions.MattesMutualInitializeSeed);

        //multi threading approach
        //m_MattesMutualInformationMetric->SetNumberOfThreads(UserOptions.NumberOfThreads);

    }

    if (metric.VIOLAWELLSMUTUALINFORMATION) {

        //set up the filters
        m_FixedGaussianFilter = DiscreteGaussianImageFitlerType::New();
        m_MovingGaussianFilter = DiscreteGaussianImageFitlerType::New();

        m_FixedNormalizeImageFilter = FixedNormalizeImageFilterType::New();
        m_MovingNormalizeImageFilter = MovingNormalizeImageFilterType::New();

        m_FixedGaussianFilter->SetVariance(2.0);
        m_MovingGaussianFilter->SetVariance(2.0);

        //pipelining the images: NormalizeImageFilter -> GaussianImageFilter -> RegistrationMethod

        m_FixedNormalizeImageFilter->SetInput(m_FixedImage);
        m_MovingNormalizeImageFilter->SetInput(m_MovingImage);

        m_FixedGaussianFilter->SetInput(m_FixedNormalizeImageFilter->GetOutput());
        m_MovingGaussianFilter->SetInput(m_MovingNormalizeImageFilter->GetOutput());

        m_ViolaWellsMutualInformationMetric->SetFixedImage(m_FixedGaussianFilter->GetOutput());
        m_ViolaWellsMutualInformationMetric->SetMovingImage(m_MovingGaussianFilter->GetOutput());
        m_ViolaWellsMutualInformationMetric->SetFixedImageRegion(m_FixedImageRegion);
        m_ViolaWellsMutualInformationMetric->SetNumberOfSpatialSamples(m_FixedImageRegion.GetNumberOfPixels()
                * UserOptions.PixelDensity);

        m_ViolaWellsMutualInformationMetric->SetFixedImageStandardDeviation(0.4);
        m_ViolaWellsMutualInformationMetric->SetMovingImageStandardDeviation(0.4);

        //m_ViolaWellsMutualInformationMetric->SetNumberOfThreads(UserOptions.NumberOfThreads);


    }
    if (metric.MUTUALINFORMATIONHISTOGRAM) {
        typename MutualInformationHistogramMetricType::HistogramSizeType histogramSize;
        histogramSize[0] = UserOptions.NumberOfBins;
        histogramSize[1] = UserOptions.NumberOfBins;
        m_MutualInformationHistogramMetric->SetHistogramSize(histogramSize);
        if (optimizer.AMOEBA) {
            m_MutualInformationHistogramMetric->ComputeGradientOff();
        }

    }

    if (metric.NORMALIZEDCORRELATION) {
        //setting up the normalized correlation metric
        m_NormalizedCorrelationMetric->SetFixedImage(m_FixedImage);
        m_NormalizedCorrelationMetric->SetMovingImage(m_MovingImage);
        m_NormalizedCorrelationMetric->SetFixedImageRegion(m_FixedImageRegion);

    }
    if (metric.MEANSQUARE) {
        m_MeanSquareMetric->SetFixedImage(m_FixedImage);
        m_MeanSquareMetric->SetMovingImage(m_MovingImage);
        m_MeanSquareMetric->SetFixedImageRegion(m_FixedImageRegion);
    }

}

template<class TFixedImageType, class TMovingImageType>
typename RegistrationFactory3D<TFixedImageType, TMovingImageType>::OutputImagePointer RegistrationFactory3D<
TFixedImageType, TMovingImageType>::GetRegisteredImage(
    void) {
    m_ResampleFilter = ResampleFilterType::New();
    m_ImageCaster = ImageCasterType::New();

    typename RegistrationMethodType::OptimizerType::ParametersType finalParameters =
        m_RegistrationObject->GetLastTransformParameters();
    m_RegistrationObject->GetTransform()->SetParameters(finalParameters);
    m_ResampleFilter->SetInput(m_MovingImage);
    m_ResampleFilter->SetTransform(m_RegistrationObject->GetTransform());
    m_ResampleFilter->SetOutputOrigin(m_FixedImage->GetOrigin());
    m_ResampleFilter->SetSize(m_FixedImage->GetLargestPossibleRegion().GetSize());
    m_ResampleFilter->SetOutputSpacing(m_FixedImage->GetSpacing());
    m_ResampleFilter->SetOutputDirection(m_FixedImage->GetDirection());
    m_ResampleFilter->SetDefaultPixelValue(0);
    m_ImageCaster->SetInput(m_ResampleFilter->GetOutput());
    m_OutputImage = m_ImageCaster->GetOutput();
    m_ImageCaster->Update();
    return m_OutputImage;
}

template<class TFixedImageType, class TMovingImageType>
typename RegistrationFactory3D<TFixedImageType, TMovingImageType>::ConstTransformBasePointer RegistrationFactory3D<
TFixedImageType, TMovingImageType>::GetTransform(
    void) {

    return m_RegistrationObject->GetOutput()->Get();
}

template<class TFixedImageType, class TMovingImageType>
typename RegistrationFactory3D<TFixedImageType, TMovingImageType>::DeformationFieldPointer RegistrationFactory3D<
TFixedImageType, TMovingImageType>::GetTransformVectorField(
    void) {
    m_DeformationField = DeformationFieldType::New();
    m_DeformationField->SetRegions(m_FixedImageRegion);
    m_DeformationField->SetOrigin(m_FixedImage->GetOrigin());
    m_DeformationField->SetSpacing(m_FixedImage->GetSpacing());
    m_DeformationField->SetDirection(m_FixedImage->GetDirection());
    m_DeformationField->Allocate();

    typedef itk::ImageRegionIterator<DeformationFieldType> DeformationFieldIteratorType;
    DeformationFieldIteratorType fi(m_DeformationField, m_FixedImageRegion);
    fi.GoToBegin();

    typename itk::Transform<double, FixedImageDimension, MovingImageDimension>::InputPointType fixedPoint;
    typename itk::Transform<double, FixedImageDimension, MovingImageDimension>::OutputPointType movingPoint;
    typename DeformationFieldType::IndexType index;

    VectorType displacement;
    while (!fi.IsAtEnd()) {
        index = fi.GetIndex();
        m_DeformationField->TransformIndexToPhysicalPoint(index, fixedPoint);
        if (transform.BSPLINEDEFORMABLETRANSFORM)
            movingPoint = m_BSplineTransform->TransformPoint(fixedPoint);
        if (transform.VERSORRIGID)
            movingPoint = m_VersorRigid3DTransform->TransformPoint(fixedPoint);
        if (transform.AFFINE)
            movingPoint = m_AffineTransform->TransformPoint(fixedPoint);
        displacement = movingPoint - fixedPoint;
        fi.Set(displacement);
        ++fi;
    }
    return m_DeformationField;

}

template<class TFixedImageType, class TMovingImageType>
typename RegistrationFactory3D<TFixedImageType, TMovingImageType>::RegistrationMethodPointer RegistrationFactory3D<
TFixedImageType, TMovingImageType>::GetRegistrationObject(
    void) {
    this->UpdateParameters();
    return m_RegistrationObject;

}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetInitialTransform(
    TransformBasePointer initialTransform) {
    const char* initialTransformName = initialTransform->GetNameOfClass();
    if (!strcmp(initialTransformName, "AffineTransform") and transform.BSPLINEDEFORMABLETRANSFORM) {
        m_BSplineTransform->SetBulkTransform(static_cast<AffineTransformType*> (initialTransform));
    }
    if (!strcmp(initialTransformName, "VersorRigid3DTransform") and transform.BSPLINEDEFORMABLETRANSFORM) {
        m_BSplineTransform->SetBulkTransform(static_cast<VersorRigid3DTransformType*> (initialTransform));
    }
    if (!strcmp(initialTransformName, "CenteredAffineTransform") and transform.BSPLINEDEFORMABLETRANSFORM) {
        m_BSplineTransform->SetBulkTransform(static_cast<CenteredAffineTransformType*> (initialTransform));
    }

    if (!strcmp(initialTransformName, "VersorRigid3DTransform") and transform.CENTEREDAFFINE) {

        m_CenteredAffineTransform->SetTranslation(
            (static_cast<VersorRigid3DTransformType*> (initialTransform)->GetTranslation()));
        m_CenteredAffineTransform->SetMatrix((static_cast<VersorRigid3DTransformType*> (initialTransform)->GetMatrix()));

    }
    if (!strcmp(initialTransformName, "VersorRigid3DTransform") and transform.AFFINE) {

        m_AffineTransform->SetTranslation(
            (static_cast<VersorRigid3DTransformType*> (initialTransform)->GetTranslation()));
        m_AffineTransform->SetMatrix((static_cast<VersorRigid3DTransformType*> (initialTransform)->GetMatrix()));

    }
    if (!strcmp(initialTransformName, "VersorRigid3DTransform") and transform.VERSORRIGID) {
        m_VersorRigid3DTransform->SetTranslation(
            (static_cast<VersorRigid3DTransformType*> (initialTransform)->GetTranslation()));
        m_VersorRigid3DTransform->SetMatrix((static_cast<VersorRigid3DTransformType*> (initialTransform)->GetMatrix()));

    }
}
/*
 this method checks the images sizes of the fixed and the moving image.
 if the fixed image size, in any direction, is bigger than the image size
 of the moving image in the respective direction, it will create a binary
 image which contains the intersection of both images

 */
template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::CheckImageSizes(
    void) {

    for (int i = 0; i < FixedImageDimension; i++) {
        if (m_FixedImageRegion.GetSize()[i] > m_MovingImageRegion.GetSize()[i]) {

            m_FixedImageIsBigger = true;
        }

    }
    if (m_FixedImageIsBigger) {
        m_MovingImageMaskObject = MaskObjectType::New();
        m_MovingThresholdFilter = MovingThresholdFilterType::New();
        m_MovingMinMaxCalculator = MovingMinMaxCalculatorType::New();

        m_MovingMinMaxCalculator->SetImage(m_MovingImage);
        m_MovingMinMaxCalculator->Compute();

        m_MovingThresholdFilter->SetInput(m_MovingImage);
        m_MovingThresholdFilter->SetOutsideValue(0);
        m_MovingThresholdFilter->SetInsideValue(255);
        m_MovingThresholdFilter->SetUpperThreshold(m_MovingMinMaxCalculator->GetMaximum());
        m_MovingThresholdFilter->SetLowerThreshold(m_MovingMinMaxCalculator->GetMinimum());
        m_MovingThresholdFilter->Update();

        m_MovingImageMaskObject->SetImage(m_MovingThresholdFilter->GetOutput());
        m_MovingImageMaskObject->Update();

    }

}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::SetFixedImageMask(
    void) {

    if (metric.MATTESMUTUALINFORMATION) {
        m_MattesMutualInformationMetric->SetFixedImageMask(m_MovingImageMaskObject);

    }
    if (metric.VIOLAWELLSMUTUALINFORMATION) {
        m_ViolaWellsMutualInformationMetric->SetFixedImageMask(m_MovingImageMaskObject);
    }
    if (metric.MUTUALINFORMATIONHISTOGRAM) {
        m_MutualInformationHistogramMetric->SetFixedImageMask(m_MovingImageMaskObject);
    }

}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::PrintResults(
    void) {
    std::cout << "Results of registration: " << std::endl << std::endl;
    if (transform.VERSORRIGID) {
        std::cout << "Versor x: " << m_RegistrationObject->GetLastTransformParameters()[0] << std::endl;
        std::cout << "Versor y: " << m_RegistrationObject->GetLastTransformParameters()[1] << std::endl;
        std::cout << "Versor z: " << m_RegistrationObject->GetLastTransformParameters()[2] << std::endl;
        std::cout << "Translation x: " << m_RegistrationObject->GetLastTransformParameters()[3] << std::endl;
        std::cout << "Translation y: " << m_RegistrationObject->GetLastTransformParameters()[4] << std::endl;
        std::cout << "Translation z: " << m_RegistrationObject->GetLastTransformParameters()[5] << std::endl;
    }
    if (optimizer.REGULARSTEPGRADIENTDESCENT) {
        std::cout << "Iterations: " << m_RegularStepGradientDescentOptimizer->GetCurrentIteration() << std::endl;
        std::cout << "Metric value: " << m_RegularStepGradientDescentOptimizer->GetValue() << std::endl;
    }
    if (optimizer.VERSORRIGID3D) {
        std::cout << "Iterations: " << m_VersorRigid3DTransformOptimizer->GetCurrentIteration() << std::endl;
        std::cout << "Metric value: " << m_VersorRigid3DTransformOptimizer->GetValue() << std::endl;

    }
    if (optimizer.LBFGSBOPTIMIZER) {
        std::cout << "Iterations: " << m_LBFGSBOptimizer->GetCurrentIteration() << std::endl;
        std::cout << "Metric value: " << m_LBFGSBOptimizer->GetValue() << std::endl;

    }
    if (optimizer.AMOEBA) {
        std::cout << "Metric value: " << m_AmoebaOptimizer->GetValue() << std::endl;

    }
}

template<class TFixedImageType, class TMovingImageType>
void RegistrationFactory3D<TFixedImageType, TMovingImageType>::StartRegistration(
    void) {

    //set all parameters to make sure all user changes are noticed
    this->UpdateParameters();
    //check the image sizes and creat a joint image mask if the fixed image is bigger than the moving image
    //to avoid a itk sample error caused by a lack of spatial samples used by the metric
    this->CheckImageSizes();

    this->SetFixedImageMask();

    if(UserOptions.SHOWITERATIONSTATUS)
    {
	    m_observer = IterationObserver::New();
	    m_RegistrationObject->GetOptimizer()->AddObserver(itk::IterationEvent(), m_observer );
    }
    
    try {
        m_RegistrationObject->StartRegistration();
    } catch (itk::ExceptionObject & err) {
        std::cerr << "isRegistrationFactory3D: Exception caught: " << std::endl << err << std::endl;
    }
    if (UserOptions.PRINTRESULTS) {
        this->PrintResults();
    }

}

} //end namespace Registration
} //end namespace isis
