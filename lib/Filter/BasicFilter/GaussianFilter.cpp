#include "GaussianFilter.hpp"
#include "common.hpp"

namespace isis
{
namespace filter
{

GaussianFilter::GaussianFilter()
{
	parameters["rowDim"] = true;
	parameters["columnDim"] = true;
	parameters["sliceDim"] = true;
	parameters["timeDim"] = false;
}

bool GaussianFilter::process( data::Image &image )
{
	ValueType sigma;

	if( !parameters["sigma"].isEmpty() && !std::isnan( parameters["sigma"].as<float>() ) ) {
		sigma = parameters["sigma"].as<ValueType>();
	} else {
		sigma = _internal::FWHM2Sigma( parameters["fwhm"].as<ValueType>() );
	}

	if( sigma <= 0 ) {
		LOG( data::Runtime, error ) << getFilterName() << ": parameter \"sigma\" has to be positive!";
		return false;
	}

	m_GaussianKernelFilter.setParameter( "sigma", sigma );
	m_GaussianKernelFilter.run();
	data::Chunk kernel = m_GaussianKernelFilter.getOutput();
	m_ConvolutionFilter.setParameter( "convolveRow", parameters["rowDim"] );
	m_ConvolutionFilter.setParameter( "convolveColumn", parameters["columnDim"] );
	m_ConvolutionFilter.setParameter( "convolveSlice", parameters["sliceDim"] );
	m_ConvolutionFilter.setParameter( "convolveTime", parameters["timeDim"] );
	m_ConvolutionFilter.setInput( "kernel", kernel );

	data::MemChunk<ValueType> ch( image.getChunk( 0 ) );

	if( !m_ConvolutionFilter.run( ch ) ) {
		return false;
	}

	data::Image im ( ch );
	image = im;
	return true;
}



}
}