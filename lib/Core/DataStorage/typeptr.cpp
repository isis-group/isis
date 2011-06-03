#include "typeptr.hpp"

namespace isis
{
namespace data
{
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
namespace _internal{


//////////////////////////////////////////////////////////    
// some voodo to get the vector types into the templates /
//////////////////////////////////////////////////////////
    
template<typename T> struct _TypeVector;
// mapping for signed types
#define DEF_VECTOR_SI(TYPE,KEY)                              \
template<> struct _TypeVector<TYPE>{                         \
	typedef TYPE vector __attribute__ ((vector_size (16)));  \
	static vector gt(vector a,vector b){return (vector)_mm_cmpgt_epi ## KEY ((__m128i)a, (__m128i)b);}                                                        \
	static vector lt(vector a,vector b){return (vector)_mm_cmplt_epi ## KEY ((__m128i)a, (__m128i)b);}                                                        \
};
DEF_VECTOR_SI( int8_t, 8);
DEF_VECTOR_SI(int16_t,16);
DEF_VECTOR_SI(int32_t,32);
    
// value offset to compare unsigned int as signed (there is no compare for unsigned in SSE2)
template<typename T> __m128i _getAddV(){
	__m128i ret;
	T *at=reinterpret_cast<T*>(&ret);
	const T filler= 1<<(sizeof(T)*8-1);
	std::fill(at, at+16/sizeof(T), filler);
	return ret;
}
    
// mapping for unsigned types
#define DEF_VECTOR_UI(TYPE,KEY)                                 \
template<> struct _TypeVector<TYPE>{                            \
	typedef TYPE vector __attribute__ ((vector_size (16)));     \
	static __m128i addv;                                        \
	static inline vector gt(vector a,vector b){return (vector)_mm_cmpgt_epi ## KEY (_mm_add_epi ## KEY((__m128i)a,addv), _mm_add_epi ## KEY((__m128i)b,addv));} \
	static inline vector lt(vector a,vector b){return (vector)_mm_cmplt_epi ## KEY (_mm_add_epi ## KEY((__m128i)a,addv), _mm_add_epi ## KEY((__m128i)b,addv));} \
};\
__m128i _TypeVector<TYPE>::addv=_getAddV<TYPE>();
    
DEF_VECTOR_UI( uint8_t, 8);
DEF_VECTOR_UI(uint16_t,16);
DEF_VECTOR_UI(uint32_t,32);

////////////////////////////////////////////    
// optimized min/max function for integers /
////////////////////////////////////////////
	
template<typename T> std::pair<T,T> _getMinMax(const T *data,size_t len){
	assert((reinterpret_cast<size_t>(data) & 0xF)==0); //make sure its 16-aligned
	LOG( Runtime, verbose_info ) << "using optimized min/max computation for " << util::Value<T>::staticName();

	size_t blocks=len/(16/sizeof(T));
	
	typedef typename _TypeVector<T>::vector block;
	const block *bdata=reinterpret_cast<const block*>(data);
	
	block min=*bdata,max=*bdata;
	while (--blocks) {
		const block &at=bdata[blocks];
		
		const block less_mask=_TypeVector<T>::lt(at, min);
		min&=~less_mask;//remove bigger values from current min
		min|=at&less_mask;//put in the lesser values from at
		
		const block greater_mask=_TypeVector<T>::gt(at, max);
		max&=~greater_mask;//remove lesser values from current min
		max|=at&greater_mask;//put in the bigger values from at
	}
	
	// compute the min/max of the blocks bmin/bmax
	const T *smin=reinterpret_cast<const T*>(&min);
	const T *smax=reinterpret_cast<const T*>(&max);
	const T bmin=*std::min_element(smin, smin+16/sizeof(T));
	const T bmax=*std::max_element(smax, smax+16/sizeof(T));
	
	// if there are some remaining elements
	blocks=len/(16/sizeof(T));
	if(data+blocks*16/sizeof(T) < data+len){
		const T rmin=*std::min_element(data+blocks*16/sizeof(T), data+len);
		const T rmax=*std::max_element(data+blocks*16/sizeof(T), data+len);
		return std::pair<T, T>(std::min(bmin,rmin),std::max(bmax,rmax)); 
	} else {
		return std::pair<T, T>(bmin,bmax);
	}
}

////////////////////////////////////////////////    
// specialize calcMinMax for (u)int(8,16,32)_t /
////////////////////////////////////////////////

template<> std::pair< uint8_t,  uint8_t> calcMinMax(const  uint8_t *data,size_t len){return _getMinMax(data, len);}
template<> std::pair<uint16_t, uint16_t> calcMinMax(const uint16_t *data,size_t len){return _getMinMax(data, len);}
template<> std::pair<uint32_t, uint32_t> calcMinMax(const uint32_t *data,size_t len){return _getMinMax(data, len);}

template<> std::pair< int8_t,  int8_t> calcMinMax(const  int8_t *data,size_t len){return _getMinMax(data, len);}
template<> std::pair<int16_t, int16_t> calcMinMax(const int16_t *data,size_t len){return _getMinMax(data, len);}
template<> std::pair<int32_t, int32_t> calcMinMax(const int32_t *data,size_t len){return _getMinMax(data, len);}

} //namepace _internal
#else
#warning Optimized min/max functions are not used because SSE2 is not enabled
#endif
    
}
}

