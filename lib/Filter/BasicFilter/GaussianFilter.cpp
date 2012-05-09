#include "DataStorage/filter.hpp"
#include "GaussianFilter.hpp"

namespace isis
{
namespace filter
{

bool GaussianFilter::process( data::Image &image )
{
	const ValueType sigma = parameterMap.getPropertyAs<ValueType>( "sigma" );

	if( sigma <= 0 ) {
		LOG( data::Runtime, error ) << getFilterName() << ": parameter \"sigma\" has to be positive!";
		return false;
	}

	m_GaussianKernelFilter.setParameter<ValueType>( "sigma", sigma );
	m_GaussianKernelFilter.run();
	data::Chunk kernel = m_GaussianKernelFilter.getOutput();

	m_ConvolutionFilter.setParameter<bool>( "convolveRow", true );
	m_ConvolutionFilter.setParameter<bool>( "convolveColumn", true );
	m_ConvolutionFilter.setParameter<bool>( "convolveSlice", true );
	m_ConvolutionFilter.setKernel( kernel );

	data::MemChunk<ValueType> ch( image.getChunk( 0 ) );

	m_ConvolutionFilter.run( ch );

	data::Image im ( ch );
	image = im;
	return true;
}



}
}