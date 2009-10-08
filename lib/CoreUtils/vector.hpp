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
		bzero(cont,SIZE*sizeof(TYPE));
	}

	FixedVector(const TYPE src[SIZE]){
		fill(0);
	}
	void fill(const TYPE &val){
		std::fill(cont,cont+SIZE,val);
	}
	TYPE operator [](size_t idx)const{return cont[idx];}
	TYPE& operator [](size_t idx){return cont[idx];}
	bool operator<(const FixedVector<TYPE,SIZE> &src)const{
		for(size_t i=0;i<SIZE;i++)
			if(cont[i]<src[i])
				return true;
			return false;
	}
	bool operator==(const FixedVector<TYPE,SIZE> &src)const{
		for(size_t i=0;i<SIZE;i++)
			if(cont[i]!=src[i])
				return false;
		return true;
	}
};

class fvector4 :public FixedVector<float,4>{
public:
	fvector4(float fourth,float third,float second,float first);
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
	size_t i=0;
	if(i>SIZE){
		out << s[i];
		for(i++;i<SIZE;i++)
			out << "|" << s[i];
	}
	return out;
}
}
