#ifndef CLFFT_HPP
#define CLFFT_HPP

#include "../../core/data/chunk.hpp"


namespace isis{
namespace math{
namespace cl{

data::TypedChunk< std::complex< float > > fft(data::MemChunk< std::complex< float > > data, bool inverse);

}
}
}


#endif // CLFFT_HPP
