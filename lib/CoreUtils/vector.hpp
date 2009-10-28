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
#include <numeric>
#include <cmath>

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{
namespace _internal{
template<typename TYPE, size_t SIZE> class array
{
protected:
	TYPE cont[SIZE];
	typedef TYPE* iterator;
	typedef const TYPE* const_iterator;
	iterator begin(){return cont;}
	const_iterator begin()const{return cont;}
	iterator end(){return cont+SIZE;}
	const_iterator end()const{return cont+SIZE;}
};
}

template<typename TYPE, size_t SIZE,typename CONTAINER=_internal::array<TYPE,SIZE> >
class FixedVector:protected CONTAINER
{
public:
	typedef typename CONTAINER::iterator iterator;
	typedef typename CONTAINER::const_iterator const_iterator;
	typedef FixedVector<TYPE,SIZE,CONTAINER> this_class;
public:

	/// Generic operations
	template<typename OP> this_class binary_op(const this_class &src)const
	{
		this_class ret;
		std::transform(CONTAINER::begin(),CONTAINER::end(),src.begin(),ret.begin(),OP());
		return ret;
	}
	template<typename OP> this_class binary_op(const TYPE &src)const
	{
		this_class ret;
		iterator dst=ret.begin();
		const OP op=OP();
		for(const_iterator i=CONTAINER::begin();i!=CONTAINER::end();i++,dst++)
			*dst=op(*i,src);
		return ret;
	}
	template<typename OP> this_class unary_op()const
	{
		this_class ret;
		std::transform(CONTAINER::begin(),CONTAINER::end(),ret.begin(),OP());
		return ret;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Contructor stuff
	////////////////////////////////////////////////////////////////////////////////////
	FixedVector(){fill(TYPE());	}

	FixedVector(const TYPE src[SIZE])
	{
		std::copy(src,src+SIZE,CONTAINER::begin());
	}

	void fill(const TYPE &val)
	{
		std::fill(CONTAINER::begin(),CONTAINER::end(),val);
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Accessors
	////////////////////////////////////////////////////////////////////////////////////
	TYPE operator [](size_t idx)const{return CONTAINER::begin()[idx];}
	TYPE& operator [](size_t idx){return CONTAINER::begin()[idx];}

	////////////////////////////////////////////////////////////////////////////////////
	// Comparison
	////////////////////////////////////////////////////////////////////////////////////
	///\returns true if this is lexically less than the given vector (first entry has highest rank)
	bool lexical_less(const this_class &src)const{
		const_iterator they=src.begin();
		const_iterator me=CONTAINER::begin();
		while(me!=CONTAINER::end()){
			if (*they<*me) return false;
			else if (*me<*they) return true;
			me++;they++;
		}
		return false;
	}
	///\returns true if this is lexically less than the given vector (first entry has lowest rank)
	bool lexical_less_reverse(const this_class &src)const{
		const_iterator they=src.end();
		const_iterator me=CONTAINER::end();
		while(me!=CONTAINER::begin()){
			me--;they--;
			if (*they<*me) return false;
			else if (*me<*they) return true;
		}
		return false;
	}
	bool operator==(const this_class &src)const{
		return std::equal(CONTAINER::begin(),CONTAINER::end(),src.begin());
	}
	bool operator!=(const this_class &src)const{
		return !operator==(src);
	}


	////////////////////////////////////////////////////////////////////////////////////
	// Arithmetic operations
	////////////////////////////////////////////////////////////////////////////////////
	this_class operator-(const this_class &src)const{return binary_op<std::minus<TYPE>      >(src);}
	this_class operator+(const this_class &src)const{return binary_op<std::plus<TYPE>       >(src);}
	this_class operator*(const this_class &src)const{return binary_op<std::multiplies<TYPE> >(src);}
	this_class operator/(const this_class &src)const{return binary_op<std::divides<TYPE>    >(src);}

	this_class operator-(const TYPE &src)const{return binary_op<std::minus<TYPE>      >(src);}
	this_class operator+(const TYPE &src)const{return binary_op<std::plus<TYPE>       >(src);}
	this_class operator*(const TYPE &src)const{return binary_op<std::multiplies<TYPE> >(src);}
	this_class operator/(const TYPE &src)const{return binary_op<std::divides<TYPE>    >(src);}

	/// \returns the inner product
	TYPE dot(const this_class &vect)const{return std::inner_product(CONTAINER::begin(),CONTAINER::end(),vect.begin(), TYPE());}
	/// \returns the inner product with itself (aka squared length)
	TYPE sqlen()const{return dot(*this);}
	/// \returns the length of the vector
	TYPE len()const{return std::sqrt(sqlen());}

	/// norms the vector (make len()==1)
	void norm()throw(std::invalid_argument)
	{
		double d= len();
		if(d==0)throw(std::invalid_argument("Trying to normalize a vector of length 0"));
		else *this = *this / d;
	}

	/// copy the data to somthing designed after the output iterator model
	template<class OutputIterator> void copyTo(OutputIterator out)
	{
		std::copy(CONTAINER::begin(),CONTAINER::end(),out);
	}

	/// write the data formated to basic_ostream
	template<typename charT, typename traits> void writeTo(std::basic_ostream<charT, traits> &out)const
	{
		util::write_list(CONTAINER::begin(),CONTAINER::end(),out,"|","<",">");
	}

};

class fvector4 :public FixedVector<float,4>{
public:
	fvector4();
	fvector4(const FixedVector<float,4> &src);
	fvector4(float first,float second,float third,float fourth);
};
}
/** @} */
}

template<typename TYPE, size_t SIZE,typename CONTAINER >
::isis::util::FixedVector<TYPE,SIZE,CONTAINER> operator-(const ::isis::util::FixedVector<TYPE,SIZE,CONTAINER>& s)
{
	return s.isis::util::FixedVector<TYPE,SIZE,CONTAINER>::template unary_op<std::negate<float> >();
}

/// Streaming output for FixedVector
namespace std{

template<typename charT, typename traits,typename TYPE, size_t SIZE,typename CONTAINER > basic_ostream<charT, traits>&
operator<<(basic_ostream<charT, traits> &out,const ::isis::util::FixedVector<TYPE,SIZE,CONTAINER>& s)
{	
	s.writeTo(out);
	return out;
}
}

#endif //VECTOR_HPP