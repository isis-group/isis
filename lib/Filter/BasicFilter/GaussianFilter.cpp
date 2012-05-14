#include "GaussianFilter.hpp"
#include "common.hpp"

namespace isis
{
namespace filter
{

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

	m_GaussianKernelFilter.parameters["sigma"] = sigma;
	m_GaussianKernelFilter.run();
	data::Chunk kernel = m_GaussianKernelFilter.getOutput();

	m_ConvolutionFilter.parameters["convolveRow"] = true;
	m_ConvolutionFilter.parameters["convolveColumn"] = true;
	m_ConvolutionFilter.parameters["convolveSlice"] = true;
	m_ConvolutionFilter.setInput( "kernel", kernel );

	data::MemChunk<ValueType> ch( image.getChunk( 0 ) );

	m_ConvolutionFilter.run( ch );

	data::Image im ( ch );
	image = im;
	return true;
}



}
}