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

Chunk::Chunk(util::_internal::TypePtrBase* src,size_t firstDim,size_t secondDim,size_t thirdDim,size_t fourthDim):
	_internal::ChunkBase(firstDim,secondDim,thirdDim,fourthDim),
	util::_internal::TypeReference<util::_internal::TypePtrBase>(src){}

boost::shared_ptr<Chunk> Chunk::cloneToNew(void *const address,size_t firstDim,size_t secondDim,size_t thirdDim,size_t fourthDim)const{
	util::FixedVector<size_t,4> newSize= sizeToVector();
	if(firstDim)newSize[0]=firstDim;
	if(secondDim)newSize[1]=secondDim;
	if(thirdDim)newSize[2]=thirdDim;
	if(fourthDim)newSize[3]=fourthDim;
	
	util::_internal::TypePtrBase* 
	  cloned(get()->cloneToNew(address,newSize.product()));
	return boost::shared_ptr<Chunk>(new Chunk(cloned,newSize[0],newSize[1],newSize[2],newSize[3]));
}
size_t Chunk::bytes_per_voxel()const{
	return get()->bytes_per_elem();
}
std::string Chunk::typeName()const{
	return get()->typeName();
}
unsigned short Chunk::typeID()const{
	return get()->typeID();
}


}}

