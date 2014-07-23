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
	parameters["physicalSpace"] = true;
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
		LOG( Runtime, error ) << getFilterName() << ": parameter \"sigma\" has to be positive!";
		return false;
	}

	util::fvector4 sigmaVec( sigma, sigma, sigma, sigma );

	if( parameters["physicalSpace"] ) {
		util::fvector4 voxelSize = image.getValueAs<util::fvector4>( "voxelSize" );

		if( image.hasProperty( "voxelGap" ) ) {
			voxelSize += image.getValueAs<util::fvector4>( "voxelGap" );
		}

		sigmaVec /= voxelSize;
	}

	m_ConvolutionFilter.setParameter( "convolveRow", parameters["rowDim"] );
	m_ConvolutionFilter.setParameter( "convolveColumn", parameters["columnDim"] );
	m_ConvolutionFilter.setParameter( "convolveSlice", parameters["sliceDim"] );
	m_ConvolutionFilter.setParameter( "convolveTime", parameters["timeDim"] );

	if( parameters["rowDim"] ) {
		m_GaussianKernelFilter.setParameter( "sigma", sigmaVec[0] );
		m_GaussianKernelFilter.run();
		m_ConvolutionFilter.setInput( "kernelRow", m_GaussianKernelFilter.getOutput() );
	}

	if( parameters["columnDim"] ) {
		m_GaussianKernelFilter.setParameter( "sigma", sigmaVec[1] );
		m_GaussianKernelFilter.run();
		m_ConvolutionFilter.setInput( "kernelColumn", m_GaussianKernelFilter.getOutput() );
	}

	if( parameters["sliceDim"] ) {
		m_GaussianKernelFilter.setParameter( "sigma", sigmaVec[2] );
		m_GaussianKernelFilter.run();
		m_ConvolutionFilter.setInput( "kernelSlice", m_GaussianKernelFilter.getOutput() );
	}

	if( parameters["timeDim"] ) {
		m_GaussianKernelFilter.setParameter( "sigma", sigmaVec[3] );
		m_GaussianKernelFilter.run();
		m_ConvolutionFilter.setInput( "kernelTime", m_GaussianKernelFilter.getOutput() );
	}

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