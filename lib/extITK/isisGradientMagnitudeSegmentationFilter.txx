/*
 * isisGradientMagnitudeSegmentationFilter.txx
 *
 *  Created on: Jun 24, 2009
 *      Author: tuerke
 */

#include "isisGradientMagnitudeSegmentationFilter.h"

namespace isis { namespace extitk {

template<class TInputImage, class TOutputImage>
GradientMagnitudeSegmentationFilter<TInputImage, TOutputImage>::GradientMagnitudeSegmentationFilter() {

	m_OutputImage = OutputImageType::New();

	m_SmoothingFilter = SmoothingFilterType::New();
	m_GradientMagnitudeFilter = GradientFilterType::New();
	m_MinMaxFilter = MinMaxFilterType::New();

	//standard parameters for smoothingfilter
	m_Sigma = 0.5;
	m_SmoothingTimeStep = 0.125;
	m_SmoothingNumberOfIterations = 5;
	m_SmoothingConductanceParameter = 9.0;
	m_UseThresholdMethod = false;

}

template<class TInputImage, class TOutputImage>
void GradientMagnitudeSegmentationFilter<TInputImage, TOutputImage>::CalculateMinMax(
		void) {

	OutputPixelType temp;

	m_MinMaxFilter->SetImage(this->GetInput());
	m_MinMaxFilter->Compute();
	m_MinOutput = m_MinMaxFilter->GetMinimum();
	m_MaxOutput = m_MinMaxFilter->GetMaximum();
	temp = m_MaxOutput - m_MinOutput;
	m_MinOutput = 0;
	m_MaxOutput = temp;
}

template<class TInputImage, class TOutputImage>
void GradientMagnitudeSegmentationFilter<TInputImage, TOutputImage>::GenerateInputRequestedRegion()
		throw (itk::InvalidRequestedRegionError) {
	// call the superclass' implementation of this method. this should
	// copy the output requested region to the input requested region
	Superclass::GenerateInputRequestedRegion();

	// get pointers to the input and output
	typename Superclass::InputImagePointer inputPtr =
			const_cast<TInputImage *> (this->GetInput());

	if (!inputPtr) {
		return;
	}

	// get a copy of the input requested region (should equal the output
	// requested region)
	typename TInputImage::RegionType inputRequestedRegion;
	inputRequestedRegion = inputPtr->GetRequestedRegion();

	// crop the input requested region at the input's largest possible region
	if (inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion())) {
		inputPtr->SetRequestedRegion(inputRequestedRegion);
		return;
	} else {
		// Couldn't crop the region (requested region is outside the largest
		// possible region).  Throw an exception.

		// store what we tried to request (prior to trying to crop)
		inputPtr->SetRequestedRegion(inputRequestedRegion);

		// build an exception
		itk::InvalidRequestedRegionError e(__FILE__, __LINE__);
		e.SetLocation(ITK_LOCATION);
		e.SetDescription(
				"Requested region is (at least partially) outside the largest possible region.");
		e.SetDataObject(inputPtr);
		throw e;
	}
}

template<class TInputImage, class TOutputImage>
void GradientMagnitudeSegmentationFilter<TInputImage, TOutputImage>::GenerateData() {

	this->CalculateMinMax();

	//smoothing filter
	m_SmoothingFilter->SetInput(this->GetInput());
	m_SmoothingFilter->SetTimeStep(m_SmoothingTimeStep);
	m_SmoothingFilter->SetNumberOfIterations(m_SmoothingNumberOfIterations);
	m_SmoothingFilter->SetConductanceParameter(m_SmoothingConductanceParameter);

	//gradient filter
	m_GradientMagnitudeFilter->SetInput(m_SmoothingFilter->GetOutput());
	m_GradientMagnitudeFilter->SetSigma(m_Sigma);

	m_GradientMagnitudeFilter->Update();
	m_OutputImage = m_GradientMagnitudeFilter->GetOutput();
	m_OutputImage->Update();

	if (m_UseThresholdMethod) {
		std::cout << "Using threshold method" << std::endl;
		m_OtsuThresholdFilter = OtsuThresholdFilterType::New();
		m_OtsuThresholdFilter->SetInput(m_OutputImage);
		m_OtsuThresholdFilter->SetOutsideValue(1);
		m_OtsuThresholdFilter->SetInsideValue(0);
		m_OtsuThresholdFilter->SetNumberOfHistogramBins(128);
		m_OtsuThresholdFilter->Update();
		this->GraftOutput(m_OtsuThresholdFilter->GetOutput());

	} else {
		this->GraftOutput(m_OutputImage);
	}
}

} //end namespace extitk
} //end namespace isis


