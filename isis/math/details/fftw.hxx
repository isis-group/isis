#include "../common.hpp"
#include "../../core/chunk.hpp"
#include "details_fft.hxx"

namespace isis{
namespace math{
namespace fftw{
namespace _internal{
void fft_impl(isis::data::TypedChunk< std::complex< double > > &data, bool inverse=false);
void fft_impl(isis::data::TypedChunk< std::complex< float > > &data, bool inverse=false);
}
template<typename T> void fft(data::TypedChunk<std::complex< T >> &data, bool inverse=false, T scale=0){
	math::_internal::halfshift(data);
	_internal::fft_impl(data,inverse);
	math::_internal::halfshift(data);

	if(scale==0)
		scale=sqrt(1./data.getVolume());

	if(scale!=1)
		for(std::complex< T > &v:data)
			v*=scale;
}
}
}
}
