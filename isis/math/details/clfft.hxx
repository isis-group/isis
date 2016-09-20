#ifndef CLFFT_HPP
#define CLFFT_HPP

#include "../../data/chunk.hpp"


namespace isis{
namespace math{
namespace cl{

void fft(data::TypedChunk< std::complex< float > > &data, bool inverse);

}
}
}


#endif // CLFFT_HPP
