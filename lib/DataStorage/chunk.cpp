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
	
ChunkBase::ChunkBase ( size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim )
{
	const size_t idx[]={firstDim,secondDim,thirdDim,fourthDim};
	init(idx);
	addNeededFromString(needed);
	if(!NDimensional<4>::volume())
		LOG(DataLog,util::error)
		<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid";
}

ChunkBase::~ChunkBase() { }

util::fvector4 ChunkBase::size()const
{
	return util::fvector4(dimSize(0),dimSize(1),dimSize(2),dimSize(3));
}

const ChunkBase::dimensions ChunkBase::dimension[ChunkBase::n_dims]={ChunkBase::read,ChunkBase::phase,ChunkBase::slice,ChunkBase::time};
}

}}

