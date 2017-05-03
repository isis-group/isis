#ifndef FFT_HPP
#define FFT_HPP

#include "../data/chunk.hpp"
#include "../data/image.hpp"


namespace isis{
namespace math{
data::Chunk fft(data::Chunk data, bool inverse=false, double scale=0);
data::Image fft(data::Image data, bool inverse=false, double scale=0);
data::TypedChunk<std::complex< float >> fft_single(isis::data::MemChunk< std::complex< float > > data, bool inverse=false, float scale=0);
data::TypedChunk<std::complex< double >> fft_double(isis::data::MemChunk< std::complex< double > > data, bool inverse=false, double scale=0);
}
}

#endif //FFT_HPP
