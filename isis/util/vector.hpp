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

	}
	/// @endcond _internal

	// operations with other vectors
	template<typename TYPE1, typename TYPE2, size_t N> std::array<TYPE1,N> &operator+=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::plus<TYPE1>>(lhs,rhs,lhs);return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N> std::array<TYPE1,N> &operator-=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::minus<TYPE1>>(lhs,rhs,lhs);return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N> std::array<TYPE1,N> &operator*=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::multiplies<TYPE1>>(lhs,rhs,lhs);return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N> std::array<TYPE1,N> &operator/=(std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) { _internal::binaryVecOp<std::divides<TYPE1>>(lhs,rhs,lhs);return lhs;}

	template<typename TYPE1, typename TYPE2, size_t N> auto operator+(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()+TYPE2()),N>
	{ std::array<decltype(TYPE1()+TYPE2()),N> ret;_internal::binaryVecOp<std::plus<decltype(TYPE1()+TYPE2())>>(lhs,rhs,ret);return ret;}

	template<typename TYPE1, typename TYPE2, size_t N> auto operator-(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()-TYPE2()),N>
	{ std::array<decltype(TYPE1()-TYPE2()),N> ret;_internal::binaryVecOp<std::minus<decltype(TYPE1()-TYPE2())>>(lhs,rhs,ret);return ret;}

	template<typename TYPE1, typename TYPE2, size_t N> auto operator*(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()*TYPE2()),N>
	{ std::array<decltype(TYPE1()*TYPE2()),N> ret;_internal::binaryVecOp<std::multiplies<decltype(TYPE1()*TYPE2())>>(lhs,rhs,ret);return ret;}

	template<typename TYPE1, typename TYPE2, size_t N> auto operator/(const std::array<TYPE1,N> &lhs, const std::array<TYPE2,N> &rhs ) -> std::array<decltype(TYPE1()/TYPE2()),N>
	{ std::array<decltype(TYPE1()/TYPE2()),N> ret;_internal::binaryVecOp<std::divides<decltype(TYPE1()/TYPE2())>>(lhs,rhs,ret);return ret;}

	// operations with scalars
	template<typename TYPE1, typename TYPE2, size_t N> std::array<TYPE1,N> &operator*=(std::array<TYPE1,N> &lhs, const TYPE2 &rhs ) {_internal::binaryOp<std::multiplies<TYPE1>>( lhs, rhs, lhs );return lhs;}
	template<typename TYPE1, typename TYPE2, size_t N> std::array<TYPE1,N> &operator/=(std::array<TYPE1,N> &lhs, const TYPE2 &rhs ) {_internal::binaryOp<std::divides<TYPE1>>( lhs, rhs, lhs );return lhs;}

	template<typename TYPE1, typename TYPE2, size_t N> auto operator*(const std::array<TYPE1,N> &lhs, const TYPE2 &rhs )  -> std::array<decltype(TYPE1()*TYPE2()),N>
	{std::array<decltype(TYPE1()*TYPE2()),N> ret;_internal::binaryOp<std::multiplies<decltype(TYPE1()*TYPE2())>>( lhs, rhs, ret );return ret;}

	template<typename TYPE1, typename TYPE2, size_t N> auto operator/(const std::array<TYPE1,N> &lhs, const TYPE2 &rhs )  -> std::array<decltype(TYPE1()*TYPE2()),N>
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
	template<typename TYPE, size_t N> void normalize(std::array<TYPE,N> &first)throw( std::invalid_argument ) {
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
		return std::accumulate(std::begin(first),std::end(first),1,
			[](const TYPE &init,const TYPE &x){return init*x;}
		);
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

//template < typename TYPE, size_t SIZE >
//class FixedVector:public std::array<TYPE, SIZE>, public boost::arithmetic<FixedVector<TYPE, SIZE>, TYPE>
//{
//public:
//	typedef FixedVector<TYPE, SIZE> this_class;
//protected:
//	/// Generic operations
//	template<typename OP> this_class& binaryOp ( const TYPE &src ){
//		const OP op = OP();
//		for (TYPE &x :*this)
//			x = op( x, src );
//		return *this;
//	}
//public:
//	////////////////////////////////////////////////////////////////////////////////////
//	// Contructor stuff
//	////////////////////////////////////////////////////////////////////////////////////
//	/**
//	 * Default constructor.
//	 * Initializes all elements with default value of TYPE.
//	 */
//	FixedVector() {fill( TYPE() ); }

//	/**
//	 * Create a FixedVector out of an array of same type and length.
//	 */
//	FixedVector( const TYPE src[SIZE] ) {
//		std::copy( src, src + SIZE, std::begin(*this) );
//	}
//	// @todo use implicit aggregate_initialization when it gets available http://en.cppreference.com/w/cpp/language/aggregate_initialization
//	FixedVector(const std::array<TYPE,SIZE> &src):std::array<TYPE,SIZE>(src){}

//	template<typename T2,size_t S2> explicit FixedVector(const std::array<T2,S2> &src){
//		std::copy(std::begin(src),std::end(src),std::begin(*this));
//	}

//	/// Set all elements to a value
//	void fill( const TYPE &val ) {
//		std::fill( std::begin(*this), std::end(*this), val );
//	}
//	size_t getBiggestVecElemAbs( ) const {
//		size_t biggestVecElem = 0;
//		TYPE tmpValue = 0;

//		for ( size_t vecElem = 0; vecElem < SIZE; vecElem++ ) {
//			if ( fabs( std::array<TYPE,SIZE>::operator[]( vecElem ) ) > fabs( tmpValue ) ) {
//				biggestVecElem = vecElem;
//				tmpValue = std::array<TYPE,SIZE>::operator[]( vecElem );
//			}
//		}

//		return biggestVecElem;
//	}

//	////////////////////////////////////////////////////////////////////////////////////
//	// Comparison
//	////////////////////////////////////////////////////////////////////////////////////
//	///\returns true if this is lexically less than the given vector (first entry has highest rank)
//	bool lexical_less( const this_class &src )const {
//		auto they = std::begin(src);
//		auto me = std::begin(*this);

//		while ( me != std::end(*this) ) {
//			if ( *they < *me ) return false;
//			else if ( *me < *they ) return true;

//			me++;
//			they++;
//		}

//		return false;
//	}
//	///\returns true if this is lexically less than the given vector (first entry has lowest rank)
//	bool lexical_less_reverse( const this_class &src )const {
//		auto they = std::end(src);
//		auto me = std::end(*this);

//		while ( me != std::begin(*this) ) {
//			me--;
//			they--;

//			if ( *they < *me ) return false;
//			else if ( *me < *they ) return true;
//		}

//		return false;
//	}
//	///\returns true if this is equal to src
//	template<typename TYPE2> bool operator==( const FixedVector<TYPE2, SIZE> &src )const {return std::equal( std::begin(*this), std::end(*this), std::begin(src) );}
//	template<typename TYPE2> bool operator!=( const FixedVector<TYPE2, SIZE> &src )const {return !this->operator==(src);}

//	////////////////////////////////////////////////////////////////////////////////////
//	// Arithmetic operations for scalars (other are defined further down outside of the class)
//	////////////////////////////////////////////////////////////////////////////////////
//	this_class& operator-=( const TYPE &src ) {return binaryOp<std::minus<TYPE>      >( src );}
//	this_class& operator+=( const TYPE &src ) {return binaryOp<std::plus<TYPE>       >( src );}
//	this_class& operator*=( const TYPE &src ) {return binaryOp<std::multiplies<TYPE> >( src );}
//	this_class& operator/=( const TYPE &src ) {return binaryOp<std::divides<TYPE>    >( src );}

//	///\returns a negated copy
//	const this_class negate()const {
//		this_class ret=*this;
//		std::transform( ret.begin(), ret.end(), ret.begin(), std::negate<TYPE>() );
//		return ret;
//	}

//	/**
//	 * Get the inner product.
//	 * \returns \f$ \overrightarrow{this} \cdot \overrightarrow{src}  = \sum_{i=0}^{SIZE-1} {this_i * src_i} \f$
//	 */
//	TYPE dot( const this_class &vect )const {return std::inner_product( std::begin(*this), std::end(*this), std::begin(vect), TYPE() );}
//	/**
//	 * Get the inner product with itself (aka squared length).
//	 * \returns \f$ \overrightarrow{this} \cdot \overrightarrow{this} = \sum_{i=0}^{SIZE-1} this_i^2 \f$
//	 */
//	TYPE sqlen()const {return dot( *this );}
//	/**
//	 * Get the the length of the vector.
//	 * \returns \f$ \sqrt{\sum_{i=0}^{SIZE-1} this_i^2} \f$
//	 */
//	TYPE len()const {return std::sqrt( sqlen() );}

//	/**
//	 * Norm the vector (make len()==1).
//	 * Applies scalar division with the result of len() to this.
//	 *
//	 * Equivalent to:
//	 * \f[ \overrightarrow{this} = \overrightarrow{this} * {1 \over {\sqrt{\sum_{i=0}^{SIZE-1} this_i^2}}}  \f]
//	 *
//	 * If len() is equal to zero std::invalid_argument will be thrown, and this wont be changed.
//	 * \returns the changed *this
//	 */
//	const this_class &norm()throw( std::invalid_argument ) {
//		const TYPE d = len();

//		if ( d == 0 )throw( std::invalid_argument( "Trying to normalize a vector of length 0" ) );
//		else *this = *this / d;

//		return *this;
//	}

//	/**
//	 * Compute the product of all elements.
//	 * \returns \f[ \prod_{i=0}^{SIZE-1} this_i \f]
//	 */
//	TYPE product()const {
//		return std::accumulate(std::begin(*this),std::end(*this),1,
//			[](const TYPE &init,const TYPE &x){return init*x;}
//		);
//	}

//	/**
//	 * Compute the sum of all elements.
//	 * \returns \f[ \sum_{i=0}^{SIZE-1} this_i \f]
//	 */
//	TYPE sum() const {
//		return std::accumulate(std::begin(*this),std::end(*this),TYPE());
//	}

//	/////////////////////////////////////////////////////////////////////////
//	// copy stuff
//	/////////////////////////////////////////////////////////////////////////
//	/// copy the elements to somthing designed after the output iterator model
//	template<class OutputIterator> void copyTo( OutputIterator out )const {
//		std::copy( std::begin(*this), std::end(*this), out );
//	}

//	/// copy the elements to somthing designed after the output iterator model
//	template<class InputIterator> void copyFrom( InputIterator iter_start, InputIterator iter_end ) {
//		LOG_IF( size_t( std::distance( iter_start, iter_end ) ) > SIZE, Runtime, error )
//				<< "Copying " << std::distance( iter_start, iter_end ) << " Elements into a vector of the size " << SIZE;
//		std::copy( iter_start, iter_end, std::begin(*this) );
//	}
//	template<typename TYPE2> FixedVector( const FixedVector<TYPE2, SIZE> &src ) {
//		src.copyTo( std::begin(*this) );
//	}
//};

template<typename TYPE> using vector3 = std::array<TYPE, 3>;
template<typename TYPE> using vector4 = std::array<TYPE, 4>;

//template<typename TYPE, size_t SIZE>
//FixedVector<TYPE, SIZE> maxVector( const FixedVector<TYPE, SIZE> &first, const FixedVector<TYPE, SIZE> &second )
//{
//	FixedVector<TYPE, SIZE> ret( first );

//	for ( size_t i = 0; i < SIZE; i++ )
//		if ( ret[i] < second[i] )ret[i] = second[i];

//	return ret;
//}
//template<typename TYPE, size_t SIZE>
//FixedVector<TYPE, SIZE> minVector( const FixedVector<TYPE, SIZE> &first, const FixedVector<TYPE, SIZE> &second )
//{
//	FixedVector<TYPE, SIZE> ret( first );

//	for ( size_t i = 0; i < SIZE; i++ )
//		if ( ret[i] > second[i] )ret[i] = second[i];

//	return ret;
//}

typedef std::array<float,4> fvector4;
typedef std::array<double,4> dvector4;
typedef std::array<float,3> fvector3;
typedef std::array<double,3> dvector3;
typedef std::array<int32_t,4> ivector4;

API_EXCLUDE_BEGIN;
/// @cond _internal
//namespace _internal {
//	template<typename OP,typename VEC1,typename VEC2> VEC1& binary_vec_op ( VEC1& rhs,const VEC2 &lhs ){
//		std::transform( std::begin(rhs), std::end(rhs), std::begin(lhs), std::begin(rhs), OP() );
//		return rhs;
//	}
	
//}
/// @endcond _internal
API_EXCLUDE_END;
}
}

// basic operators for FixedVector
// we want to return FixedVector (not std::array) to have its features when nesting operations.
// So we need the first parameter to be a FixedVector, but second can be std::array
//template<typename TYPE1, typename TYPE2, size_t SIZE> ::isis::util::FixedVector<TYPE1, SIZE> operator+( ::isis::util::FixedVector<TYPE1, SIZE> rhs,const std::array<TYPE2,SIZE> &lhs ) {return rhs+=lhs;}
//template<typename TYPE1, typename TYPE2, size_t SIZE> ::isis::util::FixedVector<TYPE1, SIZE> operator-( ::isis::util::FixedVector<TYPE1, SIZE> rhs,const std::array<TYPE2,SIZE> &lhs ) {return rhs-=lhs;}
//template<typename TYPE1, typename TYPE2, size_t SIZE> ::isis::util::FixedVector<TYPE1, SIZE> operator*( ::isis::util::FixedVector<TYPE1, SIZE> rhs,const std::array<TYPE2,SIZE> &lhs ) {return rhs*=lhs;}
//template<typename TYPE1, typename TYPE2, size_t SIZE> ::isis::util::FixedVector<TYPE1, SIZE> operator/( ::isis::util::FixedVector<TYPE1, SIZE> rhs,const std::array<TYPE2,SIZE> &lhs ) {return rhs/=lhs;}

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
