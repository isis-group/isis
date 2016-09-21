#include "../common.hpp"
#include "../../data/chunk.hpp"
#include "details_fft.hxx"

namespace isis{
namespace math{
namespace fftw{
namespace _internal{
void fft_impl(isis::data::TypedChunk< std::complex< double > > &data, bool inverse=false);
void fft_impl(isis::data::TypedChunk< std::complex< float > > &data, bool inverse=false);
}
template<typename T> void fft(data::TypedChunk<std::complex< T >> &data, bool inverse=false){
	math::_internal::halfshift(data);
	_internal::fft_impl(data,inverse);
	math::_internal::halfshift(data);

	if(inverse){ // fftw does not scale by 1/P on inverse fft, so we do it to be compatible with other implementations
		const float scale=1./data.getVolume();
		for(std::complex< T > &v:data)
			v*=scale;
	}
}
}
}
}
