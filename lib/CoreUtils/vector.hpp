//
// C++ Interface: vector
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef VECTOR_HPP
#define VECTOR_HPP

#include "CoreUtils/log.hpp"
#include "CoreUtils/type.hpp"
#include <algorithm>
#include <ostream>
#include <strings.h>

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

template<typename TYPE, size_t SIZE> class FixedVector {
	TYPE cont[SIZE];
public:
	FixedVector(){
		fill(TYPE());
	}

	FixedVector(const TYPE src[SIZE]){
		std::copy(cont,cont+SIZE,src);
	}
	
	void fill(const TYPE &val){
		std::fill(cont,cont+SIZE,val);
	}
	
	TYPE operator [](size_t idx)const{return cont[idx];}
	TYPE& operator [](size_t idx){return cont[idx];}

	///\returns true if this is lexically less than the given vector (first entry has highest rank)
	bool lexical_less(const FixedVector<TYPE,SIZE> &src)const{
		for(size_t i=0;i<SIZE;i++){
			if (src[i]<cont[i]) return false;
			else if (cont[i]<src[i]) return true;
		}
		return false;
	}
	///\returns true if this is lexically less than the given vector (first entry has lowest rank)
	bool lexical_less_reverse(const FixedVector<TYPE,SIZE> &src)const{
		for(size_t i=SIZE;i;i--){
			if (src[i-1]<cont[i-1]) return false;
			else if (cont[i-1]<src[i-1]) return true;
		}
		return false;
	}
	bool operator==(const FixedVector<TYPE,SIZE> &src)const{
		for(size_t i=0;i<SIZE;i++)
			if(cont[i]!=src[i])
				return false;
		return true;
	}
	template<class OutputIterator> void copyTo(OutputIterator out){
		std::copy(cont,cont+SIZE,out);
	}
};

class fvector4 :public FixedVector<float,4>{
public:
	fvector4(float first,float second,float third,float fourth);
	fvector4();
};
}
/** @} */
}

/// Streaming output for FixedVector
namespace std{
template<typename charT, typename traits,typename TYPE, size_t SIZE > basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out,const ::isis::util::FixedVector<TYPE,SIZE>& s)
{
	if(SIZE>0){
		out << s[0];
		for(size_t i=1;i<SIZE;i++)
			out << "|" << s[i];
	}
	return out;
}
}

#endif //VECTOR_HPP