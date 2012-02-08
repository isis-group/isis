#ifndef ENDIANESS_HPP
#define ENDIANESS_HPP

#include <boost/type_traits/is_arithmetic.hpp>
#include "../CoreUtils/types.hpp"
#include "../CoreUtils/type.hpp"

namespace isis
{
namespace data
{
namespace _internal{

// actual implementation of the scalar-byte-swap (generic version)
template<typename TYPE> TYPE doSwap(const TYPE &src){
	TYPE ret = 0;
	uint8_t *retPtr=reinterpret_cast<uint8_t*>(&ret);
	const uint8_t *srcPtr=reinterpret_cast<const uint8_t*>(&src);
	for ( unsigned short i = 0; i < sizeof(TYPE); i++ )
		retPtr[i] =  srcPtr[sizeof(TYPE)-1-i];
	return ret;
}
// specialisation (swapping 1 byte might be a bit useless)
template<> uint8_t doSwap(const uint8_t &src){return src;}
template<>  int8_t doSwap(const  int8_t &src){return src;}


// gatekeeper for doSwap (it only works with plain scalars)
template<typename TYPE,bool IS_NUM> struct EndianSwapper{
	static TYPE swap(const TYPE &src){
		LOG(Runtime,error) << "Sorry, endianess swap for " << util::Value<TYPE>::staticName() << " is not supportet";
		return src;
	}
};

// plain scalar - directly use doSwap
template<typename TYPE> struct EndianSwapper<TYPE,true>{static TYPE swap(const TYPE &src){return doSwap(src);}}; 

//color-struct - do swap for each color
template<typename TYPE> struct EndianSwapper<util::color<TYPE>,false>{ 
	static util::color<TYPE> swap(const util::color<TYPE> &src){
		util::color<TYPE> ret;
		ret.r=EndianSwapper<TYPE,boost::is_arithmetic<TYPE>::value>::swap(src.r);
		ret.g=EndianSwapper<TYPE,boost::is_arithmetic<TYPE>::value>::swap(src.g);
		ret.b=EndianSwapper<TYPE,boost::is_arithmetic<TYPE>::value>::swap(src.b);
		return ret;
	}
};

// lists - do swap for each entry
template<typename TYPE> struct EndianSwapper<std::list<TYPE>,false>{
	static std::list<TYPE> swap(const std::list<TYPE> &src){
		std::list<TYPE> ret;
		BOOST_FOREACH(const TYPE &ref,src){
			ret.push_back(EndianSwapper<TYPE,boost::is_arithmetic<TYPE>::value>::swap(ref));
		}
		return ret;
	}
};

// FixedVector - do swap for each entry
template<typename TYPE,size_t SIZE> struct EndianSwapper<util::FixedVector<TYPE,SIZE>,false>{
	static util::FixedVector<TYPE,SIZE> swap(const util::FixedVector<TYPE,SIZE> &src){
		util::FixedVector<TYPE,SIZE> ret;
		for(size_t i=0;i<SIZE;i++){
			ret[i]=EndianSwapper<TYPE,boost::is_arithmetic<TYPE>::value>::swap(src[i]);
		}
		return ret;
	}
};
} //_internal

// public interface
template<typename T> static  T endianSwap(const T& var){
	return _internal::EndianSwapper<T,boost::is_arithmetic< T >::value >::swap(var);
}
template<typename ITER,typename TITER> static  void endianSwapArray(const ITER begin,const ITER end,TITER target){
	for(ITER i=begin;i!=end;i++,target++){
		*target = endianSwap(*i);
	}
}

}
}
#endif //ENDIANESS_HPP