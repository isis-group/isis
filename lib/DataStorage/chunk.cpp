//
// C++ Implementation: chunk
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "chunk.hpp"

namespace isis{ namespace data{
namespace _internal{
	
ChunkBase::ChunkBase ( size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim ):PropertyObject(needed)
{
	MAKE_LOG(DataLog);
	const size_t idx[]={firstDim,secondDim,thirdDim,fourthDim};
	init(idx);
	if(!NDimensional<4>::volume())
		LOG(DataLog,util::error)
		<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid" << std::endl;
}

ChunkBase::~ChunkBase() { }

size_t ChunkBase::size ( size_t index )const
{
	return NDimensional<4>::dimSize(index);
}
util::fvector4 ChunkBase::size()const
{
	return util::fvector4(size(0),size(1),size(2),size(3));
}

size_t ChunkBase::volume()const
{
	return NDimensional<4>::volume();
}

}

}}

