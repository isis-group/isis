#include "chunk.hpp"

namespace isis{ namespace data{
namespace _internal{
	
ChunkBase::ChunkBase ( size_t fourthDim, size_t thirdDim, size_t secondDim, size_t firstDim )
{
	MAKE_LOG(DataLog);
	const size_t idx[]={firstDim,secondDim,thirdDim,fourthDim};
	init(idx);
	if(!size())
		LOG(DataLog,isis::util::error)
		<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid" << std::endl;
}

ChunkBase::~ChunkBase() { }

}
	
}}

