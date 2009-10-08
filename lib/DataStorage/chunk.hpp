//
// C++ Interface: chunk
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef CHUNK_H
#define CHUNK_H

#include "CoreUtils/type.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>
#include "ndimensional.h"
#include "propertyobject.h"

namespace isis{ 
/*! \addtogroup data
*  Additional documentation for group `mygrp'
*  @{
*/

namespace data{

namespace _internal{
class ChunkBase :protected NDimensional<4>,public PropertyObject{
protected:
	static const isis::util::PropMap::key_type needed[];
public:
	typedef isis::util::_internal::TypeReference <ChunkBase > Reference;
	ChunkBase(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim);
	virtual ~ChunkBase(); //needed to make it polymorphic
};
}
	
/**
 * Main class for four-dimensional random-access data blocks.
 */
template<typename TYPE> class Chunk : public ::isis::util::TypePtr<TYPE>, public _internal::ChunkBase{
protected:
	/**
	 * Creates an data-block from existing data.
	 * \param src is a pointer to the existing data. This data will automatically be deleted. So don't use this pointer afterwards.
	 * \param d is the deleter to be used for deletion of src. It must define operator(TYPE *), which than shall free the given pointer.
	 */
	template<typename D> Chunk(TYPE* src,D d,size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim):
	_internal::ChunkBase(fourthDim,thirdDim,secondDim,firstDim),
	::isis::util::TypePtr<TYPE>(src,size(),d)
	{
	}
public:
	/**
	Returns reference to the element at a given index.
	If index is invalid, behaviour is undefined. Most probably it will crash.
	If _ENABLE_DATA_DEBUG is true an error message will be send (but it will still crash).
	*/
	TYPE &operator()(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim){
		MAKE_LOG(DataDebug);
		const size_t idx[]={firstDim,secondDim,thirdDim,fourthDim};
		if(!rangeCheck(idx)){
			LOG(DataDebug,isis::util::error)
				<< "Index " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim
				<< " is out of range (" << sizeToString() << ")"
				<< std::endl;
		}
		return this->operator[](dim2Index(idx));
	}
};

/// @cond _internal
namespace _internal{
class ChunkReference : public _internal::ChunkBase::Reference {
	template<typename T> ::isis::util::TypePtr<T> &getTypePtr(){
		MAKE_LOG(DataLog);
		boost::shared_ptr<isis::util::_internal::TypePtrBase>
		const ptr(boost::dynamic_pointer_cast<isis::util::_internal::TypePtrBase>(*this));
		if(!ptr)
			LOG(DataLog,isis::util::error)
			<< "Cannot cast this ChunkReference to TypePtrBase." << std::endl;
		return ptr->cast_to_TypePtr<T>();
	}
public:
	template<typename T> ChunkReference(const Chunk<T> &src) :	ChunkBase::Reference(new Chunk<T>(src)){}
	template<typename T> Chunk<T>& getChunk(){
		return dynamic_cast<Chunk<T>& >(getTypePtr<T>());
	}
};

struct binary_chunk_comarison : public std::binary_function< ChunkReference, ChunkReference, bool>{
	virtual bool operator() (const ChunkReference& a, const ChunkReference& b)=0;
};
}
/// @endcond

class ChunkList:public std::list< _internal::ChunkReference>{
public:
	template<typename T> void push_back(const Chunk<T> &chunk){
		std::list< _internal::ChunkReference>::push_back(_internal::ChunkReference(chunk));
	}
};

/**
 * Chunk class for memory-based buffers
 */
template<typename TYPE> class MemChunk : public Chunk<TYPE>{
public:
	MemChunk(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim): 
	Chunk<TYPE>(
		(TYPE*)malloc(sizeof(TYPE)*fourthDim*thirdDim*secondDim*firstDim),
		typename ::isis::util::TypePtr<TYPE>::BasicDeleter(),
		fourthDim,thirdDim,secondDim,firstDim
	){}
};

}
/** @} */
}
#endif // CHUNK_H
