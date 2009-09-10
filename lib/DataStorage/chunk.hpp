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
#include "common.hpp"

namespace isis{ namespace data{
	
namespace _internal{

template<typename T> class ChunkBase : public ::isis::util::TypePtr<T>{
	size_t fourthSize,thirdSize,secondSize,firstSize;
	inline size_t dim2index(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim){
		return	firstDim + secondDim*firstSize + thirdDim*secondSize*firstSize+
				fourthDim*thirdSize*secondSize*firstSize;
	}
protected:
	template<typename D> ChunkBase(T* src,D d,size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim): 
	::isis::util::TypePtr<T>(src,fourthDim*thirdDim*secondDim*firstDim,d),
	fourthSize(fourthDim),thirdSize(thirdDim),secondSize(secondDim),firstSize(firstDim){
		MAKE_LOG(DataDebug);
		if(!(fourthSize && thirdSize && secondSize && firstSize)){
			LOG(DataDebug,isis::util::error) << "Chunksize (" << fourthSize << "x" << thirdSize << "x" << secondSize << "x" << firstSize  << ") not valid" << std::endl;
		}
	}
public:
	T &operator()(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim){
		MAKE_LOG(DataDebug);
		if(fourthDim>=fourthSize || thirdDim>=thirdSize || secondDim>=secondSize || firstDim>=firstSize){
			LOG(DataDebug,isis::util::error) 
				<< "Index " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim 
				<< " is out of range (" << fourthSize << "x" << thirdSize << "x" << secondSize << "x" << firstSize  << ")" 
				<< std::endl;
		}
		return this->operator[](dim2index(fourthDim,thirdDim,secondDim,firstDim));
	}
	template<typename S> bool copyTo(const ChunkBase<S> &src){
		//@todo implement me
	}
};

}

template<typename T> class MemChunk : public _internal::ChunkBase<T>{
public:
	MemChunk(size_t fourthDim,size_t thirdDim,size_t secondDim,size_t firstDim): 
	_internal::ChunkBase<T>( 
		(T*)malloc(sizeof(T)*fourthDim*thirdDim*secondDim*firstDim),
		typename ::isis::util::TypePtr<T>::BasicDeleter(),
		fourthDim,thirdDim,secondDim,firstDim
	){}
};
}}
#endif // CHUNK_H
