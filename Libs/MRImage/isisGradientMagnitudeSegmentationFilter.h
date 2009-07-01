/*
 * isisFastMarchingSegmentationFilter.h
 *
 *  Created on: Jun 24, 2009
 *      Author: tuerke
 */

#ifndef ISISFASTMARCHINGSEGMENTATIONFILTER_H_
#define ISISFASTMARCHINGSEGMENTATIONFILTER_H_

#include "itkImageToImageFilter.h"

//segmentation related includes
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"


#include "itkMinimumMaximumImageCalculator.h"

#include "itkOtsuThresholdImageFilter.h"




namespace isis {

template< class TInputImage, class TOutputImage >
class ITK_EXPORT GradientMagnitudeSegmentationFilter :
public itk::ImageToImageFilter< TInputImage, TOutputImage >
{
public:

	itkStaticConstMacro( InputImageDimension, unsigned int,
			TInputImage::ImageDimension );
	itkStaticConstMacro( OutputImageDimension, unsigned int,
			TOutputImage::ImageDimension );

	//standard typedefs
	typedef GradientMagnitudeSegmentationFilter		Self;
	typedef itk::SmartPointer< Self >				Pointer;
	typedef itk::SmartPointer< const Self >			ConstPointer;
	typedef itk::ImageToImageFilter< TInputImage, TOutputImage >
													Superclass;

	itkNewMacro( Self );

	itkTypeMacro( GradientMagnitudeSegmentationFilter, itk::ImageToImageFilter );

	typedef TInputImage								InputImageType;
	typedef TOutputImage							OutputImageType;

	typedef typename InputImageType::PixelType		InputPixelType;
	typedef typename OutputImageType::PixelType		OutputPixelType;
	typedef float									InternalPixelType;

	typedef typename InputImageType::DirectionType	InputDirectionType;
	typedef typename OutputImageType::DirectionType OutputDirectionType;

	typedef typename InputImageType::PointType		InputPointType;
	typedef typename OutputImageType::PointType		OutputPointType;

	typedef typename InputImageType::RegionType		InputRegionType;
	typedef typename OutputImageType::RegionType	OutputRegionType;

	typedef typename InputImageType::IndexType		InputIndexType;
	typedef typename OutputImageType::IndexType		OutputIndexType;

	typedef typename InputImageType::SizeType		InputSizeType;
	typedef typename OutputImageType::SizeType		OutputSizeType;

	typedef typename InputImageType::SpacingType	InputSpacingType;
	typedef typename OutputImageType::SpacingType	OutputSpacingType;


	typedef typename InputImageType::ConstPointer	InputImageConstPointer;
	typedef typename OutputImageType::Pointer		OutputImagePointer;


	typedef typename itk::CurvatureAnisotropicDiffusionImageFilter
			< InputImageType, OutputImageType >		SmoothingFilterType;

	typedef typename itk::GradientMagnitudeRecursiveGaussianImageFilter
			< OutputImageType, OutputImageType >		GradientFilterType;

	typedef typename itk::MinimumMaximumImageCalculator
			< InputImageType >						MinMaxFilterType;

	typedef typename itk::OtsuThresholdImageFilter
			< OutputImageType, OutputImageType >	OtsuThresholdFilterType;



	itkSetMacro( Sigma, float );

	itkSetMacro( SmoothingTimeStep, float );
	itkSetMacro( SmoothingNumberOfIterations, int );
	itkSetMacro( SmoothingConductanceParameter, float );
	itkSetMacro( UseThresholdMethod, bool );


	GradientMagnitudeSegmentationFilter();
	virtual ~GradientMagnitudeSegmentationFilter() {}
	void GenerateData();
	virtual void GenerateInputRequestedRegion() throw(itk::InvalidRequestedRegionError);



private:

	void CalculateMinMax( void );


	typename OtsuThresholdFilterType::Pointer		m_OtsuThresholdFilter;



	OutputImagePointer								m_OutputImage;
	InputImageConstPointer							m_InputImage;

	InputRegionType									m_InputRegion;
	OutputRegionType								m_OutputRegion;

	InputDirectionType								m_InputDirection;
	OutputDirectionType								m_OutputDirection;

	InputPointType									m_InputOrigin;
	OutputPointType									m_OutputOrigin;

	InputIndexType									m_InputIndex;
	OutputIndexType									m_OutputIndex;

	InputSizeType									m_InputSize;
	OutputSizeType									m_OutputSize;

	InputSpacingType								m_InputSpacing;
	OutputSpacingType								m_OutputSpacing;

	typename SmoothingFilterType::Pointer			m_Smoothing;
	typename GradientFilterType::Pointer			m_GradientMagnitude;

	typename MinMaxFilterType::Pointer				m_MinMaxFilter;



	OutputPixelType									m_MinOutput;
	OutputPixelType									m_MaxOutput;



	float											m_Sigma;

	float											m_SmoothingTimeStep;
	int 											m_SmoothingNumberOfIterations;
	float											m_SmoothingConductanceParameter;

	bool											m_UseThresholdMethod;

};

}
#if ITK_TEMPLATE_TXX
#include "isisGradientMagnitudeSegmentationFilter.txx"
#endif /* ITK_TEMPLATE_TXX */
#endif /* ISISFASTMARCHINGSEGMENTATIONFILTER_H_ */
