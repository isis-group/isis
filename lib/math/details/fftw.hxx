#include "../common.hpp"
#include "../../core/data/chunk.hpp"
#include "details_fft.hxx"

namespace isis{
namespace math{
namespace fftw{
namespace _internal{
data::TypedChunk<std::complex< double >> fft_impl(data::MemChunk<std::complex< double >> data, bool inverse=false);
data::TypedChunk<std::complex< float >> fft_impl(data::MemChunk<std::complex< float >> data, bool inverse=false);
}
template<typename T> data::TypedChunk<std::complex< T >> fft(data::MemChunk<std::complex< T >> data, bool inverse=false){
	math::_internal::halfshift(data);
	data::TypedChunk<std::complex< T >> ret=_internal::fft_impl(data,inverse);
	math::_internal::halfshift(ret);

	if(inverse){ // fftw does not scale by 1/P on inverse fft, so we do it to be compatible with other implementations
		const float scale=1./data.getVolume();
		for(std::complex< T > &v:ret)
			v*=scale;
	}
	return ret;
}
}
}
}
