/*
 * isisTimeStepExtractionFilter.txx
 *
 *  Created on: May 15, 2009
 *      Author: tuerke
 */
#ifndef __TIMESTEPEXTRACTIONFILTER_TXX
#define __TIMESTEPEXTRACTIONFILTER_TXX

#include "isisTimeStepExtractionFilter.h"

namespace isis {

template< class TInputImage, class TOutputImage >
TimeStepExtractionFilter< TInputImage, TOutputImage >
::TimeStepExtractionFilter()
{
	m_ExtractFilter = ExtractFilterType::New();
	m_RequestedTimeStep = 0;
	m_RequestedTimeRangeBegin = 0;
	m_RequestedTimeRangeEnd = 0;

}


template <class TInputImage, class TOutputImage>
void
TimeStepExtractionFilter<TInputImage,TOutputImage>
::GenerateOutputInformation()
{
  // do not call the superclass' implementation of this method since
  // this filter allows the input and the output to be of different dimensions

  // get pointers to the input and output
  typename Superclass::OutputImagePointer      outputPtr = this->GetOutput();
  typename Superclass::InputImageConstPointer  inputPtr  = this->GetInput();

  if ( !outputPtr || !inputPtr)
    {
    return;
    }
  m_ExtractionRegion = inputPtr->GetLargestPossibleRegion();

  // Set the output image size to the same value as the extraction region.
  outputPtr->SetLargestPossibleRegion( m_OutputRegion );

  // Set the output spacing and origin
  const itk::ImageBase<InputImageDimension> *phyData;

  phyData
    = dynamic_cast<const itk::ImageBase<InputImageDimension>*>(this->GetInput());

  if (phyData)
    {
    // Copy what we can from the image from spacing and origin of the input
    // This logic needs to be augmented with logic that select which
    // dimensions to copy

    unsigned int i;
    const typename InputImageType::SpacingType&
      inputSpacing = inputPtr->GetSpacing();
    std::cout << "Input spacing: " << inputSpacing << std::endl;
    const typename InputImageType::DirectionType&
      inputDirection = inputPtr->GetDirection();
    const typename InputImageType::PointType&
      inputOrigin = inputPtr->GetOrigin();

    typename OutputImageType::SpacingType outputSpacing;
    typename OutputImageType::DirectionType outputDirection;
    typename OutputImageType::PointType outputOrigin;

    if ( static_cast<unsigned int>(OutputImageDimension) >
         static_cast<unsigned int>(InputImageDimension )    )
      {
      // copy the input to the output and fill the rest of the
      // output with zeros.
      for (i=0; i < InputImageDimension; ++i)
        {
        outputSpacing[i] = inputSpacing[i];
        outputOrigin[i] = inputOrigin[i];
        for (unsigned int dim = 0; dim < InputImageDimension; ++dim)
          {
          outputDirection[i][dim] = inputDirection[i][dim];
          }
        }
      for (; i < OutputImageDimension; ++i)
        {
        outputSpacing[i] = 1.0;
        outputOrigin[i] = 0.0;
        for (unsigned int dim = 0; dim < InputImageDimension; ++dim)
          {
          outputDirection[i][dim] = 0.0;
          }
        outputDirection[i][i] = 1.0;
        }
      }
    else
      {
      // copy the non-collapsed part of the input spacing and origing to the output
      outputDirection.SetIdentity();
      int nonZeroCount = 0;
      for (i=0; i < InputImageDimension; ++i)

        {

        if (m_ExtractionRegion.GetSize()[i])
          {
          outputSpacing[nonZeroCount] = inputSpacing[i];

          outputOrigin[nonZeroCount] = inputOrigin[i];
          int nonZeroCount2 = 0;
          for (unsigned int dim = 0; dim < InputImageDimension; ++dim)
            {

            if (m_ExtractionRegion.GetSize()[dim])

              {
              outputDirection[nonZeroCount][nonZeroCount2] =
                inputDirection[nonZeroCount][dim];
              ++nonZeroCount2;
              }
            }
          nonZeroCount++;
          }
        }
      }
    // This is a temporary fix to get over the problems with using OrientedImages to
    // a non-degenerate extracted image to be created.  It still needs to be determined
    // how to compute the correct outputDirection from all possible input directions.
    if (vnl_determinant(outputDirection.GetVnlMatrix()) == 0.0)
      {
      outputDirection.SetIdentity();
      }

    // set the spacing and origin

    outputPtr->SetSpacing( outputSpacing );
    outputPtr->SetDirection( outputDirection );
    outputPtr->SetOrigin( outputOrigin );
    outputPtr->SetNumberOfComponentsPerPixel(
    inputPtr->GetNumberOfComponentsPerPixel() );
    }
  else
    {
    // pointer could not be cast back down
    itkExceptionMacro(<< "itk::ExtractImageFilter::GenerateOutputInformation "
                      << "cannot cast input to "
                      << typeid(itk::ImageBase<InputImageDimension>*).name() );
    }
}




template< class TInputImage, class TOutputImage >
void
TimeStepExtractionFilter< TInputImage, TOutputImage>
::GenerateData()
{

	if ( m_RequestedTimeRangeBegin == 0 and m_RequestedTimeRangeEnd == 0 )
	{
	//extraction of a single timestep if m_RequestedTimeRangeBegin and m_RequestedTimeRangeEnd not set
		m_InputRegion = this->GetInput()->GetLargestPossibleRegion();
		m_InputSize = m_InputRegion.GetSize();
		m_InputIndex = m_InputRegion.GetIndex();
		//this collapses the last dimension so the output dimension is input dimension-1
		m_InputSize[ OutputImageDimension ] = 0;
		m_InputIndex[ OutputImageDimension ] = m_RequestedTimeStep;
		m_DesiredRegion.SetSize( m_InputSize );
		m_DesiredRegion.SetIndex( m_InputIndex );
		m_ExtractFilter->SetExtractionRegion( m_DesiredRegion );
		m_ExtractFilter->SetInput( this->GetInput() );
		try
			{
				m_ExtractFilter->Update();
			}
			catch( itk::ExceptionObject & err )
			{
				std::cerr << "ExceptionObject caught !" << std::endl;
				std::cerr << err << std::endl;
			}
		this->GraftOutput( m_ExtractFilter->GetOutput() );

	}
	else
	{
		if ( m_RequestedTimeRangeEnd - m_RequestedTimeRangeBegin > 1 )
		{
			//extraction of a certain timespan set by m_RequestedTimeRangeBegin and m_RequestedTimeRangeBegin
			//notice that the timespan has to be bigger than 1
			std::cout << "extracting time range " << m_RequestedTimeRangeBegin
			<< " to " << m_RequestedTimeRangeEnd << std::endl;
			m_InputRegion = this->GetInput()->GetLargestPossibleRegion();
			m_InputSize = m_InputRegion.GetSize();
			m_InputIndex = m_InputRegion.GetIndex();

			m_InputSize[ OutputImageDimension - 1] = m_RequestedTimeRangeEnd - m_RequestedTimeRangeBegin;
			m_InputIndex[ OutputImageDimension - 1] = m_RequestedTimeRangeBegin;

			m_DesiredRegion.SetSize( m_InputSize );
			m_DesiredRegion.SetIndex( m_InputIndex );

			m_ExtractFilter->SetExtractionRegion( m_DesiredRegion );
			m_ExtractFilter->SetInput( this->GetInput() );
			try
				{
					m_ExtractFilter->Update();
				}
				catch( itk::ExceptionObject & err )
				{
					std::cerr << "ExceptionObject caught !" << std::endl;
					std::cerr << err << std::endl;
				}
			this->GraftOutput( m_ExtractFilter->GetOutput() );




		}
		else
		{
			std::cout << "range has to be bigger than 1" << std::endl;

		}

	}



}

/*
template< class TInputImage, class TOutputImage >
void
TimeStepExtractorFilter< TInputImage, TOutputImage >
::Start( void )
{
	typename ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();



	//this collapses the last dimension


	extractFilter->SetExtractionRegion( m_OutputRegion );
	extractFilter->SetInput( m_InputImage );
	try
	{
		extractFilter->Update();
	}
	catch( itk::ExceptionObject & err )
	{
		std::cerr << "ExceptionObject caught !" << std::endl;
		std::cerr << err << std::endl;
	}
	outputPtr = extractFilter->GetOutput();
}

*/
}// end namspace isis
#endif
