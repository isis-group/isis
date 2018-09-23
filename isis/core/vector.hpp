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

#ifdef _MSC_VER
#pragma warning(disable:4290)
#endif

#include "common.hpp"
#include <algorithm>
#include <ostream>
#include <numeric>
#include <cmath>
#include <type_traits>

#include <array>
#include <boost/operators.hpp>

namespace isis
{
	/// @cond _internal
	namespace _internal
	{
	struct VectorClass{};//empty base class to recognize vectors
	/// Generic operations
	template<typename OP,typename TYPE1, typename TYPE2, typename TYPE3, size_t N>
	void binaryVecOp(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs, std::array<TYPE3,N> &dst){
		std::transform( std::begin(lhs), std::end(lhs), std::begin(rhs), std::begin(dst), OP() );
	}
	template<typename OP,typename TYPE1, typename TYPE2, typename TYPE3, size_t N>
	void binaryOp ( const std::array<TYPE1,N> &lhs, const TYPE2 &rhs, std::array<TYPE3,N> &dst ){
		const OP op = OP();
		auto src=std::begin(lhs);
		for (TYPE3 &x :dst)
			x = op( *(src++), rhs);
	}
	template<typename TYPE1,typename TYPE2> using scalar_only = 
		typename std::enable_if< std::is_scalar<TYPE1>::value && std::is_scalar<TYPE2>::value,int>;
	}
	/// @endcond _internal

	// operations with other vectors
	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> std::array<TYPE1,N>&
	operator+=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::plus<TYPE1>>(lhs,rhs,lhs);return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> std::array<TYPE1,N>&
	operator-=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::minus<TYPE1>>(lhs,rhs,lhs);return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> std::array<TYPE1,N>&
	operator*=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::multiplies<TYPE1>>(lhs,rhs,lhs);return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> std::array<TYPE1,N>&
	operator/=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::divides<TYPE1>>(lhs,rhs,lhs);return lhs;}

	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> auto 
	operator+(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()+TYPE2()),N>
	{ std::array<decltype(TYPE1()+TYPE2()),N> ret;_internal::binaryVecOp<std::plus<decltype(TYPE1()+TYPE2())>>(lhs,rhs,ret);return ret;}

	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> auto 
	operator-(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()-TYPE2()),N>
	{ std::array<decltype(TYPE1()-TYPE2()),N> ret;_internal::binaryVecOp<std::minus<decltype(TYPE1()-TYPE2())>>(lhs,rhs,ret);return ret;}

	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> auto 
	operator*(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()*TYPE2()),N>
	{ std::array<decltype(TYPE1()*TYPE2()),N> ret;_internal::binaryVecOp<std::multiplies<decltype(TYPE1()*TYPE2())>>(lhs,rhs,ret);return ret;}

	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> auto 
	operator/(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()/TYPE2()),N>
	{ std::array<decltype(TYPE1()/TYPE2()),N> ret;_internal::binaryVecOp<std::divides<decltype(TYPE1()/TYPE2())>>(lhs,rhs,ret);return ret;}

	// operations with scalars
	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> std::array<TYPE1,N>&
	operator*=(std::array<TYPE1,N> &lhs, const TYPE2 &rhs ) {_internal::binaryOp<std::multiplies<TYPE1>>( lhs, rhs, lhs );return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> std::array<TYPE1,N>&
	operator/=(std::array<TYPE1,N> &lhs, const TYPE2 &rhs ) {_internal::binaryOp<std::divides<TYPE1>>( lhs, rhs, lhs );return lhs;}

	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> auto 
	operator*(const std::array<TYPE1,N> &lhs, const TYPE2 &rhs)  -> std::array<decltype(TYPE1()*TYPE2()),N>
	{std::array<decltype(TYPE1()*TYPE2()),N> ret;_internal::binaryOp<std::multiplies<decltype(TYPE1()*TYPE2())>>( lhs, rhs, ret );return ret;}

	template<typename TYPE1, typename TYPE2, size_t N, typename _internal::scalar_only<TYPE1,TYPE2>::type=0> auto 
	operator/(const std::array<TYPE1,N> &lhs, const TYPE2 &rhs )  -> std::array<decltype(TYPE1()/TYPE2()),N>
	{std::array<decltype(TYPE1()/TYPE2()),N> ret;_internal::binaryOp<std::divides<decltype(TYPE1()/TYPE2())>>( lhs, rhs, ret );return ret;}

	template<typename TYPE, size_t N>
	std::array<TYPE, N> operator-( std::array<TYPE, N> s )
	{
		std::transform( std::begin(s), std::end(s), std::begin(s), std::negate<TYPE>() );
		return s;
	}


namespace util
{
	/**
	 * Get the inner product.
	 * \returns \f$ \overrightarrow{this} \cdot \overrightarrow{src}  = \sum_{i=0}^{SIZE-1} {this_i * src_i} \f$
	 */
	template<typename TYPE1, typename TYPE2, size_t N> auto dot( const std::array<TYPE1,N> &first, const std::array<TYPE2,N> &second) -> decltype(TYPE1()*TYPE2())
	{return std::inner_product( std::begin(first), std::end(first), std::begin(second), decltype(TYPE1()*TYPE2())() );}
	/**
	 * Get the inner product with itself (aka squared length).
	 * \returns \f$ \overrightarrow{this} \cdot \overrightarrow{this} = \sum_{i=0}^{SIZE-1} this_i^2 \f$
	 */
	template<typename TYPE1, size_t N> TYPE1 sqlen( const std::array<TYPE1,N> &first){return dot( first,first );}
	/**
	 * Get the the length of the vector.
	 * \returns \f$ \sqrt{\sum_{i=0}^{SIZE-1} this_i^2} \f$
	 */
	template<typename TYPE1, size_t N> TYPE1 len(const std::array<TYPE1,N> &first){return std::sqrt( sqlen(first) );}
	/**
	 * Normalize the vector (make len()==1).
	 * Applies scalar division with the result of len() to this.
	 *
	 * Equivalent to:
	 * \f[ \overrightarrow{this} = \overrightarrow{this} * {1 \over {\sqrt{\sum_{i=0}^{SIZE-1} this_i^2}}}  \f]
	 *
	 * If len() is equal to zero std::invalid_argument will be thrown, and this wont be changed.
	 * \returns the changed *this
	 */
	template<typename TYPE, size_t N> void normalize(std::array<TYPE,N> &first) {
		const TYPE d = len(first);

		if ( d == 0 )throw( std::invalid_argument( "Trying to normalize a vector of length 0" ) );
		else first /= d;
	}

	/**
	 * Fuzzy comparison for vectors.
	 * Does util::fuzzyEqual for the associated elements of the two vectors.
	 * @param other the "other" vector to compare to
	 * @param scale scaling factor forwarded to util::fuzzyEqual
	 * \returns true if util::fuzzyEqual for all elements
	 */
	template<typename TYPE1, typename TYPE2, size_t N>
	bool fuzzyEqualV( const std::array<TYPE1,N> &first, const std::array<TYPE2,N> &second, unsigned short scale = 10 ){
		auto b = std::begin(second);

		for ( TYPE1 a : first ) {
			if ( ! util::fuzzyEqual( a, *(b++), scale ) )
				return false;
		}

		return true;
	}

	/**
	 * Compute the product of all elements.
	 * \returns \f[ \prod_{i=0}^{SIZE-1} this_i \f]
	 */
	template<typename TYPE, size_t N>
	TYPE product(const std::array<TYPE,N> &first){
		TYPE ret=1;
		for(const TYPE &t:first)
			ret*=t;
		return ret;
	}

	/**
	 * Compute the sum of all elements.
	 * \returns \f[ \sum_{i=0}^{SIZE-1} this_i \f]
	 */
	template<typename TYPE, size_t N>
	TYPE sum(const std::array<TYPE,N> &first) {
		return std::accumulate(std::begin(first),std::end(first),TYPE());
	}


	template<typename TYPE1, typename TYPE2, size_t N>
	bool lexical_less_reverse( const std::array<TYPE1,N> &first, const std::array<TYPE2,N> &second){
		auto they = std::end(second);
		auto me = std::end(first);

		while ( me != std::begin(first) ) {
			--me;
			--they;

			if ( *they < *me ) return false;
			else if ( *me < *they ) return true;
		}

		return false;
	}

template<typename TYPE> using vector3 = std::array<TYPE, 3>;
template<typename TYPE> using vector4 = std::array<TYPE, 4>;

typedef std::array<float,4> fvector4;
typedef std::array<double,4> dvector4;
typedef std::array<int32_t,4> ivector4;
typedef std::array<float,3> fvector3;
typedef std::array<double,3> dvector3;
typedef std::array<int32_t,3> ivector3;

}
}

/// Streaming output for FixedVector
namespace std
{

template<typename charT, typename traits, typename TYPE, size_t SIZE> basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const std::array<TYPE, SIZE>& s )
{
	isis::util::listToOStream( std::begin(s), std::end(s), out, "|", "<", ">" );
	return out;
}
}
#endif //VECTOR_HPP
