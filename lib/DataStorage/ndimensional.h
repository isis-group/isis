/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <2009>  <Enrico Reimer>

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

#ifndef NDIMENSIONAL_H
#define NDIMENSIONAL_H

#define	__need_size_t
#include <stddef.h>
#include <algorithm>
#include <boost/static_assert.hpp>
#include <string>
#include <sstream>
#include "CoreUtils/common.hpp"

namespace isis{ namespace data{ namespace _internal{

template<unsigned short DIM> size_t __dimStride(const size_t dim[])
{
	BOOST_STATIC_ASSERT(DIM>0);//Make sure recursion terminates
	return __dimStride<DIM-1>(dim)*dim[DIM-1];
}

template<unsigned short DIM> size_t __dim2Index(const size_t d[],const size_t dim[])
{
	BOOST_STATIC_ASSERT(DIM>0);//Make sure recursion terminates
	return d[DIM]*__dimStride<DIM>(dim)+__dim2Index<DIM-1>(d,dim);
}

template<unsigned short DIM> bool __rangeCheck(const size_t d[],const size_t dim[])
{
	return (d[DIM] < dim[DIM]) && __rangeCheck<DIM-1>(d,dim);
}

template<> size_t __dimStride<0> (                 const size_t dim[]);
template<> size_t __dim2Index<0> (const size_t d[],const size_t dim[]);
template<> bool   __rangeCheck<0>(const size_t d[],const size_t dim[]);

/// Base class for anything that has dimensional size
template<unsigned short SIZE> class NDimensional{
	size_t dim[SIZE];
protected:
	NDimensional(){}
public:
	void init(const size_t d[SIZE]){
		std::copy(d,d+SIZE,dim);
		//@todo make validity check
	}
	NDimensional(const NDimensional &src){
		init(src.dim);
	}
	/**
	Compute linear index from n-dimensional index,
	\param d array of indexes (d[0] is most iterating element / lowest dimension)
	*/
	size_t dim2Index(const size_t d[SIZE])const
	{
		return __dim2Index<SIZE-1>(d,dim);
	}
	bool rangeCheck(const size_t d[SIZE]){
		return __rangeCheck<SIZE-1>(d,dim);
	}
	size_t size(){
	  return __dimStride<SIZE>(dim);
	}
	/// generates a string representing the size
	std::string sizeToString(std::string delim="x"){
		std::ostringstream ret;
		size_t rev[SIZE];
		std::reverse_copy(dim,dim+SIZE,rev);
		isis::util::write_list(rev,rev+SIZE,ret,delim);
		return ret.str();
	}
};

}}}

#endif // NDIMENSIONAL_H
