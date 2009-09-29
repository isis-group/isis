/*
 * isisSTDEVMaskFilter.txx
 *
 *  Created on: Jun 23, 2009
 *      Author: tuerke
 */

#include "isisSTDEVMaskFilter.h"

namespace isis {

template<class TInputImage, class TOutputImage>
STDEVMaskFilter<TInputImage, TOutputImage>::STDEVMaskFilter() {

	m_InputImage = InputImageType::New();
	m_OutputImage = OutputImageType::New();
	m_Begin = 1;
}

template<class TInputImage, class TOutputImage>
void STDEVMaskFilter<TInputImage, TOutputImage>::SetOutputParameters() {
	m_InputImage = this->GetInput();
	m_InputRegion = m_InputImage->GetBufferedRegion();

	m_InputIndex = m_InputRegion.GetIndex();
	m_InputSize = m_InputRegion.GetSize();
	m_InputSpacing = m_InputImage->GetSpacing();
	m_InputOrigin = m_InputImage->GetOrigin();

	for (unsigned int i = 0; i < OutputImageDimension; i++) {
		m_OutputSize[i] = m_InputSize[i];
		m_OutputIndex[i] = m_InputIndex[i];
		m_OutputSpacing[i] = m_InputSpacing[i];
		m_OutputOrigin[i] = m_InputOrigin[i];
	}

	m_OutputImage->SetSpacing(m_OutputSpacing);
	m_OutputImage->SetOrigin(m_OutputOrigin);
	m_OutputRegion.SetSize(m_OutputSize);
	m_OutputRegion.SetIndex(m_OutputIndex);

	m_OutputImage->SetRegions(m_OutputRegion);
	m_OutputImage->Allocate();

	m_Timelength = m_InputRegion.GetSize()[OutputImageDimension];

}

template<class TInputImage, class TOutputImage>
void STDEVMaskFilter<TInputImage, TOutputImage>::Update() {
	float dev;
	unsigned int ts;

	this->SetOutputParameters();
	IteratorType m_Iterator(m_InputImage, m_InputRegion);
	m_Iterator.SetDirection(OutputImageDimension);
	m_Iterator.GoToBegin();
	while (!m_Iterator.IsAtEnd()) {
		SumType q = itk::NumericTraits<SumType>::Zero;
		SumType sum = itk::NumericTraits<SumType>::Zero;
		m_Iterator.GoToBeginOfLine();
		m_InputIndex = m_Iterator.GetIndex();
		ts = 0;
		while (!m_Iterator.IsAtEndOfLine()) {

			ts++;
			if (ts > m_Begin) {

				sum += m_Iterator.Get();
				q += m_Iterator.Get() * m_Iterator.Get();
			}
			//std::cout << "q: " << q << std::endl;

			++m_Iterator;

		}
		//MeanType mean = static_cast< MeanType >( sum ) /
		//			static_cast< MeanType >( m_Timelength );
		dev = sqrt((((m_Timelength - m_Begin) * q) - (sum * sum))
				/ ((m_Timelength - m_Begin) * (m_Timelength - (m_Begin + 1))));
		sum = 0;
		q = 0;
		//std::cout << dev << std::endl;

		for (unsigned int i = 0; i < OutputImageDimension; i++) {
			m_OutputIndex[i] = m_InputIndex[i];
		}
		m_OutputImage->SetPixel(m_OutputIndex,
				static_cast<OutputPixelType> (dev));
		m_Iterator.NextLine();
	}
}

template<class TInputImage, class TOutputImage>
typename STDEVMaskFilter<TInputImage, TOutputImage>::OutputImagePointer STDEVMaskFilter<
		TInputImage, TOutputImage>::GetOutput(void) {
	return m_OutputImage;
}

} //end namespace isis
