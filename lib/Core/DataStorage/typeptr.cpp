#include "typeptr.hpp"

namespace isis
{
namespace data
{
/// @cond _hidden
// specialisation for complex - there shall be no scaling - and we cannot compute minmax
template<> scaling_pair ValuePtr<std::complex<float> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const
{
	return scaling_pair( util::Value<float>( 1 ), util::Value<float>( 0 ) );
}
template<> scaling_pair ValuePtr<std::complex<double> >::getScalingTo( unsigned short /*typeID*/, autoscaleOption /*scaleopt*/ )const
{
	return scaling_pair( util::Value<double>( 1 ), util::Value<double>( 0 ) );
}

#ifdef __SSE2__

#include <emmintrin.h>
namespace _internal
{


///////////////////////////////////////////////////////////
// some voodoo to get the vector types into the templates /
//////////////////////////////////////////////////////////
template<typename T> struct _VectorUnion {
	union {__m128i reg; T elem[16 / sizeof( T )];} vec;
	_VectorUnion( const T *el ) {std::copy( el, el + 16 / sizeof( T ), vec.elem );}
	_VectorUnion( __m128i _reg ) {vec.reg = _reg;}
	_VectorUnion() {}
};
template<typename T> struct _TypeVector;

// mapping for signed types
#define DEF_VECTOR_SI(TYPE,KEY)                              \
	template<> struct _TypeVector<TYPE>{                         \
		static __m128i gt(__m128i a,__m128i b){return _mm_cmpgt_epi ## KEY (a, b);}                                                        \
		static __m128i lt(__m128i a,__m128i b){return _mm_cmplt_epi ## KEY (a, b);}                                                        \
	};
DEF_VECTOR_SI( int8_t, 8 );
DEF_VECTOR_SI( int16_t, 16 );
DEF_VECTOR_SI( int32_t, 32 );

// value offset to compare unsigned int as signed (there is no compare for unsigned in SSE2)
template<typename T> __m128i _getAddV()
{
	_VectorUnion<T> ret;
	const T filler = 1 << ( sizeof( T ) * 8 - 1 );
	std::fill( ret.vec.elem, ret.vec.elem + 16 / sizeof( T ), filler );
	return ret.vec.reg;
}

// mapping for unsigned types
#define DEF_VECTOR_UI(TYPE,KEY)                                 \
	template<> struct _TypeVector<TYPE>{                            \
		static __m128i addv;                                        \
		static inline __m128i gt(__m128i a,__m128i b){return _mm_cmpgt_epi ## KEY (_mm_add_epi ## KEY(a,addv), _mm_add_epi ## KEY(b,addv));} \
		static inline __m128i lt(__m128i a,__m128i b){return _mm_cmplt_epi ## KEY (_mm_add_epi ## KEY(a,addv), _mm_add_epi ## KEY(b,addv));} \
	};\
	__m128i _TypeVector<TYPE>::addv=_getAddV<TYPE>();

DEF_VECTOR_UI( uint8_t, 8 );
DEF_VECTOR_UI( uint16_t, 16 );
DEF_VECTOR_UI( uint32_t, 32 );



////////////////////////////////////////////
// optimized min/max function for integers /
////////////////////////////////////////////

// generic fallback using cmpgt and some bitmask voodoo
template<typename T> std::pair<__m128i, __m128i> _getMinMaxBlockLoop( const __m128i *data, size_t blocks )
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<T>::staticName() << " (masked mode)";
	static const __m128i one = _mm_set_epi16( 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF );

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );

		const __m128i less_mask = _TypeVector<T>::lt( at, ret.first );
		const __m128i greater_mask = _TypeVector<T>::gt( at, ret.second );

		ret.first =
			( ret.first & ( less_mask ^ one ) ) //remove bigger values from current min
			| ( at & less_mask ); //put in the lesser values from at
		ret.second =
			( ret.second & ( greater_mask ^ one ) ) //remove lesser values from current max
			| ( at & greater_mask ); //put in the bigger values from at
	}

	return ret;
}

// specialiced versions using processor opcodes for min/max
template<> std::pair<__m128i, __m128i> _getMinMaxBlockLoop<uint8_t>( const __m128i *data, size_t blocks ) //PMAXUB
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<uint8_t>::staticName() << " (direct mode)";

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );
		ret.first = _mm_min_epu8( ret.first, at );
		ret.second = _mm_max_epu8( ret.second, at );
	}

	return ret;
}
template<> std::pair<__m128i, __m128i> _getMinMaxBlockLoop<int16_t>( const __m128i *data, size_t blocks ) //PMAXSW
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<int16_t>::staticName() << " (direct mode)";

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );
		ret.first = _mm_min_epi16( ret.first, at );
		ret.second = _mm_max_epi16( ret.second, at );
	}

	return ret;
}

#ifdef __SSE4_1__
#include <smmintrin.h>
template<> std::pair<__m128i, __m128i> _getMinMaxBlockLoop<int8_t>( const __m128i *data, size_t blocks ) //PMAXSB
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<int8_t>::staticName() << " (direct mode)";

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );
		ret.first = _mm_min_epi8( ret.first, at );
		ret.second = _mm_max_epi8( ret.second, at );
	}

	return ret;
}
template<> std::pair<__m128i, __m128i> _getMinMaxBlockLoop<uint16_t>( const __m128i *data, size_t blocks ) //PMAXUW
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<uint16_t>::staticName() << " (direct mode)";

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );
		ret.first = _mm_min_epu16( ret.first, at );
		ret.second = _mm_max_epu16( ret.second, at );
	}

	return ret;
}
template<> std::pair<__m128i, __m128i> _getMinMaxBlockLoop<int32_t>( const __m128i *data, size_t blocks ) //PMAXSD
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<int32_t>::staticName() << " (direct mode)";

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );
		ret.first = _mm_min_epi32( ret.first, at );
		ret.second = _mm_max_epi32( ret.second, at );
	}

	return ret;
}
template<> std::pair<__m128i, __m128i> _getMinMaxBlockLoop<uint32_t>( const __m128i *data, size_t blocks ) //PMAXSD
{
	std::pair<__m128i, __m128i> ret( _mm_loadu_si128( data ), _mm_loadu_si128( data ) );
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<uint32_t>::staticName() << " (direct mode)";

	while ( --blocks ) {
		const __m128i at = _mm_loadu_si128( ++data );
		ret.first = _mm_min_epu32( ret.first, at );
		ret.second = _mm_max_epu32( ret.second, at );
	}

	return ret;
}
#endif

template<typename T> std::pair<T, T> _getMinMax( const T *data, size_t len )
{
	size_t blocks = len / ( 16 / sizeof( T ) );

	T bmin = std::numeric_limits<T>::max();
	T bmax = std::numeric_limits<T>::min();

	if( blocks ) { // if there are 16byte-blocks of values
		std::pair<__m128i, __m128i> minmax = _getMinMaxBlockLoop<T>( reinterpret_cast<const __m128i *>( data ), blocks );
		// compute the min/max of the blocks bmin/bmax
		const _VectorUnion<T> smin = minmax.first;
		const _VectorUnion<T> smax = minmax.second;
		bmin = *std::min_element( smin.vec.elem, smin.vec.elem + 16 / sizeof( T ) );
		bmax = *std::max_element( smax.vec.elem, smax.vec.elem + 16 / sizeof( T ) );
	}

	// if there are some remaining elements
	if( data + blocks * 16 / sizeof( T ) < data + len ) {
		const T rmin = *std::min_element( data + blocks * 16 / sizeof( T ), data + len );
		const T rmax = *std::max_element( data + blocks * 16 / sizeof( T ), data + len );
		return std::pair<T, T>( std::min( bmin, rmin ), std::max( bmax, rmax ) );
	} else {
		return std::pair<T, T>( bmin, bmax );
	}
}

////////////////////////////////////////////////
// specialize calcMinMax for (u)int(8,16,32)_t /
////////////////////////////////////////////////

template<> std::pair< uint8_t,  uint8_t> calcMinMax( const  uint8_t *data, size_t len ) {return _getMinMax( data, len );}
template<> std::pair<uint16_t, uint16_t> calcMinMax( const uint16_t *data, size_t len ) {return _getMinMax( data, len );}
template<> std::pair<uint32_t, uint32_t> calcMinMax( const uint32_t *data, size_t len ) {return _getMinMax( data, len );}

template<> std::pair< int8_t,  int8_t> calcMinMax( const  int8_t *data, size_t len ) {return _getMinMax( data, len );}
template<> std::pair<int16_t, int16_t> calcMinMax( const int16_t *data, size_t len ) {return _getMinMax( data, len );}
template<> std::pair<int32_t, int32_t> calcMinMax( const int32_t *data, size_t len ) {return _getMinMax( data, len );}

} //namepace _internal
#else
#warning Optimized min/max functions are not used because SSE2 is not enabled
#endif
/// @endcond
}
}

