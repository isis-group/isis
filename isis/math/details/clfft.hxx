#ifndef CLFFT_HPP
#define CLFFT_HPP

#include "../../data/chunk.hpp"


namespace isis{
namespace math{
namespace cl{

void fft(data::TypedChunk< std::complex< float > > &data, bool inverse, float scale=0);

}
}
}


#endif // CLFFT_HPP
