/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHUNK_H
#define CHUNK_H

#include "CoreUtils/type.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/propmap.hpp"
#include "common.hpp"
#include <string.h>
#include <list>

namespace isis{ 
/*! \addtogroup data
*  Additional documentation for group `mygrp'
*  @{
*/

namespace data{

/**
 * Main class for four-dimensional random-access data blocks.
 */
template<typename TYPE> class Chunk : public ::isis::util::TypePtr<TYPE>{
	size_t fourthSize,thirdSize,secondSize,firstSize;
	inline size_t dim2index(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim)const{
		return	firstDim + secondDim*firstSize + thirdDim*secondSize*firstSize+
				fourthDim*thirdSize*secondSize*firstSize;
	}
protected:
	/**
	 * Creates an data-block from existing data.
	 * \param src is a pointer to the existing data. This data will automatically be deleted. So don't use this pointer afterwards.
	 * \param d is the deleter to be used for deletion of src. It must define operator(TYPE *), which than shall free the given pointer.
	 */
	template<typename D> Chunk(TYPE* src,D d,size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim):
	::isis::util::TypePtr<TYPE>(src,fourthDim*thirdDim*secondDim*firstDim,d),
	fourthSize(fourthDim),thirdSize(thirdDim),secondSize(secondDim),firstSize(firstDim){
		MAKE_LOG(DataDebug);
		if(!(fourthSize && thirdSize && secondSize && firstSize)){
			LOG(DataDebug,isis::util::error) << "Chunksize (" << fourthSize << "x" << thirdSize << "x" << secondSize << "x" << firstSize  << ") not valid" << std::endl;
		}
	}
public:
	/**
	Returns reference to the element at a given index.
	If index is invalid, behaviour is undefined. Most probably it will crash.
	If _ENABLE_DATA_DEBUG is true an error message will be send (but it will still crash).
	*/
	TYPE &operator()(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim){
		MAKE_LOG(DataDebug);
		if(fourthDim>=fourthSize || thirdDim>=thirdSize || secondDim>=secondSize || firstDim>=firstSize){
			LOG(DataDebug,isis::util::error) 
				<< "Index " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim 
				<< " is out of range (" << fourthSize << "x" << thirdSize << "x" << secondSize << "x" << firstSize  << ")" 
				<< std::endl;
		}
		return this->operator[](dim2index(fourthDim,thirdDim,secondDim,firstDim));
	}
	::isis::util::PropMap properties;
};

/// @cond _internal
namespace _internal{
class ChunkReference : public ::isis::util::_internal::TypePtrBase::Reference {
public:
	template<typename T> ChunkReference(const Chunk<T> &src) : ::isis::util::_internal::TypePtrBase::Reference(new Chunk<T>(src)){}
	
	template<typename T> Chunk<T>& getAs(){
		return dynamic_cast<Chunk<T>& >((*this)->cast_to_TypePtr<T>());
	}
};
}
/// @endcond

class ChunkList:public std::list< _internal::ChunkReference>{
public:
	template<typename T> iterator add(const Chunk<T> &chunk){
		push_back(_internal::ChunkReference(chunk));
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
