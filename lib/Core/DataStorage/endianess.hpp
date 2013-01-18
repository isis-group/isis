#ifndef ENDIANESS_HPP
#define ENDIANESS_HPP

#include <boost/type_traits/is_arithmetic.hpp>
#include "../CoreUtils/types.hpp"
#include "../CoreUtils/value.hpp"
#include <byteswap.h>

namespace isis
{
namespace data
{
/// @cond _internal
namespace _internal
{

// actual implementation of the scalar-byte-swap (generic version)
template<uint_fast8_t SIZE> struct SwapImpl {
	template<typename TYPE> static TYPE doSwap( const TYPE &src ) {
		TYPE ret = 0;
		uint8_t *retPtr = reinterpret_cast<uint8_t *>( &ret );
		const uint8_t *srcPtr = reinterpret_cast<const uint8_t *>( &src );

		for ( uint_fast8_t i = 0; i < SIZE; i++ )
			retPtr[i] =  srcPtr[SIZE - 1 - i];

		return ret;
	}
};

// specialisation (swapping 1 byte might be a bit useless)
template<> struct SwapImpl<1> {
	template<typename TYPE> static TYPE doSwap( const TYPE &src ) {return src;}
};

//specializations for 16 32 and 64 bit
template<> struct SwapImpl<2> {
	template<typename TYPE> static TYPE doSwap( const TYPE &src ) {return __bswap_16( src );}
};
template<> struct SwapImpl<4> {
	template<typename TYPE> static TYPE doSwap( const TYPE &src ) {return __bswap_32( src );}
};
template<> struct SwapImpl<8> {
	template<typename TYPE> static TYPE doSwap( const TYPE &src ) {return __bswap_64( src );}
};



// gatekeeper for SwapImpl (do not allow swapping of types we do not know about)
template<typename TYPE, bool IS_NUM> struct EndianSwapper {
	static TYPE swap( const TYPE &src ) {
		LOG( Runtime, error ) << "Sorry, endianess swap for " << util::Value<TYPE>::staticName() << " is not supportet";
		return src;
	}
};

// plain numeric scalar - directly use doSwap
template<typename TYPE> struct EndianSwapper<TYPE, true> {
	static TYPE swap( const TYPE &src ) {return SwapImpl<sizeof( TYPE )>::doSwap( src );}
};

//color-struct - do swap for each color
template<typename TYPE> struct EndianSwapper<util::color<TYPE>, false> {
	static util::color<TYPE> swap( const util::color<TYPE> &src ) {
		util::color<TYPE> ret;
		ret.r = EndianSwapper<TYPE, boost::is_arithmetic<TYPE>::value>::swap( src.r );
		ret.g = EndianSwapper<TYPE, boost::is_arithmetic<TYPE>::value>::swap( src.g );
		ret.b = EndianSwapper<TYPE, boost::is_arithmetic<TYPE>::value>::swap( src.b );
		return ret;
	}
};

// lists - do swap for each entry
template<typename TYPE> struct EndianSwapper<std::list<TYPE>, false> {
	static std::list<TYPE> swap( const std::list<TYPE> &src ) {
		std::list<TYPE> ret;
		std::transform( src.begin(), src.end(), ret.begin(), EndianSwapper<TYPE, boost::is_arithmetic<TYPE>::value>::swap );
		return ret;
	}
};

// FixedVector - do swap for each entry
template<typename TYPE, size_t SIZE> struct EndianSwapper<util::FixedVector<TYPE, SIZE>, false> {
	static util::FixedVector<TYPE, SIZE> swap( const util::FixedVector<TYPE, SIZE> &src ) {
		util::FixedVector<TYPE, SIZE> ret;
		std::transform( src.begin(), src.end(), ret.begin(), EndianSwapper<TYPE, boost::is_arithmetic<TYPE>::value>::swap );
		return ret;
	}
};
} //_internal
/// @endcond _internal

// public interface
template<typename T> static  T endianSwap( const T &var )
{
	return _internal::EndianSwapper<T, boost::is_arithmetic< T >::value >::swap( var );
}
template<typename ITER, typename TITER> static  void endianSwapArray( const ITER begin, const ITER end, TITER target )
{
	for( ITER i = begin; i != end; i++, target++ ) {
		*target = endianSwap( *i );
	}
}

}
}
#endif //ENDIANESS_HPP
