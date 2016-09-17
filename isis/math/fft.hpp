#ifndef FFT_HPP
#define FFT_HPP

#include "../data/chunk.hpp"


namespace isis{
namespace math{
data::Chunk fft(data::Chunk data, bool inverse=false);
data::TypedChunk<std::complex< float >> fft_single(isis::data::MemChunk< std::complex< float > > data, bool inverse=false);
data::TypedChunk<std::complex< double >> fft_double(isis::data::MemChunk< std::complex< double > > data, bool inverse=false);
}
}

#endif //FFT_HPP
