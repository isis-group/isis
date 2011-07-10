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

namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util
{
namespace _internal
{
template<typename TYPE, size_t SIZE> class array
{
protected:
	TYPE cont[SIZE];
	typedef TYPE *iterator;
	typedef const TYPE *const_iterator;
	iterator begin() {return cont;}
	const_iterator begin()const {return cont;}
	iterator end() {return cont + SIZE;}
	const_iterator end()const {return cont + SIZE;}
};
}

template < typename TYPE, size_t SIZE, typename CONTAINER = _internal::array<TYPE, SIZE> >
class FixedVector: protected CONTAINER
{
public:
	typedef typename CONTAINER::iterator iterator;
	typedef typename CONTAINER::const_iterator const_iterator;
	typedef FixedVector<TYPE, SIZE, CONTAINER> this_class;
protected:
	/// Generic operations
	template<typename OP> this_class binaryOp ( const this_class &src )const {
		this_class ret;
		std::transform( CONTAINER::begin(), CONTAINER::end(), src.begin(), ret.begin(), OP() );
		return ret;
	}
	template<typename OP> this_class binaryOp( const TYPE &src )const {
		this_class ret;
		iterator dst = ret.begin();
		const OP op = OP();

		for ( const_iterator i = CONTAINER::begin(); i != CONTAINER::end(); i++, dst++ )
			*dst = op( *i, src );

		return ret;
	}
	template<typename OP> this_class unaryOp()const {
		this_class ret;
		std::transform( CONTAINER::begin(), CONTAINER::end(), ret.begin(), OP() );
		return ret;
	}
public:
	////////////////////////////////////////////////////////////////////////////////////
	// Contructor stuff
	////////////////////////////////////////////////////////////////////////////////////
	/**
	 * Default constructor.
	 * Initializes all elements with default value of TYPE.
	 */
	FixedVector() {fill( TYPE() ); }

	/**
	 * Create a FixedVector out of an array of same type and length.
	 */
	FixedVector( const TYPE src[SIZE] ) {
		std::copy( src, src + SIZE, CONTAINER::begin() );
	}

	/// Set all elements to a value
	void fill( const TYPE &val ) {
		std::fill( CONTAINER::begin(), CONTAINER::end(), val );
	}
	size_t getBiggestVecElemAbs( ) const {
		size_t biggestVecElem = 0;
		TYPE tmpValue = 0;

		for ( size_t vecElem = 0; vecElem < SIZE; vecElem++ ) {
			if ( fabs( operator[]( vecElem ) ) > fabs( tmpValue ) ) {
				biggestVecElem = vecElem;
				tmpValue = operator[]( vecElem );
			}
		}

		return biggestVecElem;
	}
	////////////////////////////////////////////////////////////////////////////////////
	// Accessors
	////////////////////////////////////////////////////////////////////////////////////
	const TYPE &operator []( size_t idx )const {
		LOG_IF( idx >= SIZE, Debug, error ) << "Index " << idx << " exceeds the size of the vector (" << SIZE << ")";
		return CONTAINER::begin()[idx];
	}
	TYPE &operator []( size_t idx ) {
		LOG_IF( idx >= SIZE, Debug, error ) << "Index " << idx << " exceeds the size of the vector (" << SIZE << ")";
		return CONTAINER::begin()[idx];
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Comparison
	////////////////////////////////////////////////////////////////////////////////////
	///\returns true if this is lexically less than the given vector (first entry has highest rank)
	bool lexical_less( const this_class &src )const {
		const_iterator they = src.begin();
		const_iterator me = CONTAINER::begin();

		while ( me != CONTAINER::end() ) {
			if ( *they < *me ) return false;
			else if ( *me < *they ) return true;

			me++;
			they++;
		}

		return false;
	}
	///\returns true if this is lexically less than the given vector (first entry has lowest rank)
	bool lexical_less_reverse( const this_class &src )const {
		const_iterator they = src.end();
		const_iterator me = CONTAINER::end();

		while ( me != CONTAINER::begin() ) {
			me--;
			they--;

			if ( *they < *me ) return false;
			else if ( *me < *they ) return true;
		}

		return false;
	}
	///\returns true if this is equal to src
	bool operator==( const this_class &src )const {return std::equal( CONTAINER::begin(), CONTAINER::end(), src.begin() );}
	///\returns false if this is equal to src
	bool operator!=( const this_class &src )const {
		return !operator==( src );
	}
	/**
	 * Fuzzy comparison.
	 * Will raise a compiler error when not used with floating point vectors.
	 * @param other the other vector that should be compared with the current vector.
	 * @param thresh a threshold factor to set a minimal difference to be still considered equal independent of the values itself.
	 * Eg. "1" means any difference less than the epsilon of the used floating point type will allways be considered equal.
	 * If any of the values is greater than "1" the "allowed" difference will be bigger.
	 * \returns true if the difference between the two types is significantly small compared to the values.
	 */
	bool fuzzyEqual( const this_class &other, TYPE thresh = 0 )const {
		const_iterator b = other.begin();

		for ( const_iterator a = CONTAINER::begin(); a != CONTAINER::end(); ++a, ++b ) {
			if ( ! util::fuzzyEqual( *a, *b, thresh ) )
				return false;
		}

		return true;
	}



	////////////////////////////////////////////////////////////////////////////////////
	// Arithmetic operations
	////////////////////////////////////////////////////////////////////////////////////
	this_class operator-( const this_class &src )const {return binaryOp<std::minus<TYPE>      >( src );}
	this_class operator+( const this_class &src )const {return binaryOp<std::plus<TYPE>       >( src );}
	this_class operator*( const this_class &src )const {return binaryOp<std::multiplies<TYPE> >( src );}
	this_class operator/( const this_class &src )const {return binaryOp<std::divides<TYPE>    >( src );}

	this_class operator-( const TYPE &src )const {return binaryOp<std::minus<TYPE>      >( src );}
	this_class operator+( const TYPE &src )const {return binaryOp<std::plus<TYPE>       >( src );}
	this_class operator*( const TYPE &src )const {return binaryOp<std::multiplies<TYPE> >( src );}
	this_class operator/( const TYPE &src )const {return binaryOp<std::divides<TYPE>    >( src );}

	///\returns a negated copy
	const this_class negate()const {
		return unaryOp<std::negate<float> >();
	}

	/**
	 * Get the inner product.
	 * \returns \f$ \overrightarrow{this} \cdot \overrightarrow{src}  = \sum_{i=0}^{SIZE-1} {this_i * src_i} \f$
	 */
	TYPE dot( const this_class &vect )const {return std::inner_product( CONTAINER::begin(), CONTAINER::end(), vect.begin(), TYPE() );}
	/**
	 * Get the inner product with itself (aka squared length).
	 * \returns \f$ \overrightarrow{this} \cdot \overrightarrow{this} = \sum_{i=0}^{SIZE-1} this_i^2 \f$
	 */
	TYPE sqlen()const {return dot( *this );}
	/**
	 * Get the the length of the vector.
	 * \returns \f$ \sqrt{\sum_{i=0}^{SIZE-1} this_i^2} \f$
	 */
	TYPE len()const {return std::sqrt( sqlen() );}

	/**
	 * Norm the vector (make len()==1).
	 * Applies scalar division with the result of len() to this.
	 *
	 * Equivalent to:
	 * \f[ \overrightarrow{this} = \overrightarrow{this} * {1 \over {\sqrt{\sum_{i=0}^{SIZE-1} this_i^2}}}  \f]
	 *
	 * If len() is equal to zero std::invalid_argument will be thrown, and this wont be changed.
	 * \returns the changed *this
	 */
	const this_class &norm()throw( std::invalid_argument ) {
		const TYPE d = len();

		if ( d == 0 )throw( std::invalid_argument( "Trying to normalize a vector of length 0" ) );
		else *this = *this / d;

		return *this;
	}

	/**
	 * Compute the product of all elements.
	 * \returns \f[ \prod_{i=0}^{SIZE-1} this_i \f]
	 */
	TYPE product()const {
		TYPE ret = 1;

		for ( const_iterator i = CONTAINER::begin(); i != CONTAINER::end(); i++ )
			ret *= *i;

		return ret;
	}

	/**
	 * Compute the sum of all elements.
	 * \returns \f[ \sum_{i=0}^{SIZE-1} this_i \f]
	 */
	TYPE sum() {
		TYPE ret = 0;

		for ( iterator i = CONTAINER::begin(); i != CONTAINER::end(); i++ )
			ret += *i;

		return ret;
	}

	/////////////////////////////////////////////////////////////////////////
	// copy stuff
	/////////////////////////////////////////////////////////////////////////
	/// copy the elements to somthing designed after the output iterator model
	template<class OutputIterator> void copyTo( OutputIterator out )const {
		std::copy( CONTAINER::begin(), CONTAINER::end(), out );
	}

	/// copy the elements to somthing designed after the output iterator model
	template<class InputIterator> void copyFrom( InputIterator iter_start, InputIterator iter_end ) {
		LOG_IF( size_t( std::distance( iter_start, iter_end ) ) > SIZE, Runtime, error )
				<< "Copying " << std::distance( iter_start, iter_end ) << " Elements into a vector of the size " << SIZE;
		std::copy( iter_start, iter_end, CONTAINER::begin() );
	}
	template<typename TYPE2, typename CONTAINER2> FixedVector( const FixedVector<TYPE2, SIZE, CONTAINER2> &src ) {
		src.copyTo( CONTAINER::begin() );
	}

	/// write the elements formated to basic_ostream
	template<typename charT, typename traits> void writeTo( std::basic_ostream<charT, traits> &out )const {
		util::listToOStream( CONTAINER::begin(), CONTAINER::end(), out, "|", "<", ">" );
	}

};

template<typename TYPE>
class vector4 : public FixedVector<TYPE, 4>
{
public:
	vector4() {}
	template<typename TYPE2, typename CONTAINER2> vector4( const FixedVector<TYPE2, 4, CONTAINER2> &src ) : FixedVector< TYPE, 4> ( src ) {}
	vector4( TYPE first, TYPE second, TYPE third = 0, TYPE fourth = 0 ) {
		this->operator[]( 3 ) = fourth;
		this->operator[]( 2 ) = third;
		this->operator[]( 1 ) = second;
		this->operator[]( 0 ) = first;
	}
};

template<typename TYPE, size_t SIZE, typename CONTAINER1, typename CONTAINER2>
FixedVector<TYPE, SIZE> maxVector( const FixedVector<TYPE, SIZE, CONTAINER1> &first, const FixedVector<TYPE, SIZE, CONTAINER2> &second )
{
	FixedVector<TYPE, SIZE> ret( first );

	for ( size_t i = 0; i < SIZE; i++ )
		if ( ret[i] < second[i] )ret[i] = second[i];

	return ret;
}
template<typename TYPE, size_t SIZE, typename CONTAINER1, typename CONTAINER2>
FixedVector<TYPE, SIZE> minVector( const FixedVector<TYPE, SIZE, CONTAINER1> &first, const FixedVector<TYPE, SIZE, CONTAINER2> &second )
{
	FixedVector<TYPE, SIZE> ret( first );

	for ( size_t i = 0; i < SIZE; i++ )
		if ( ret[i] > second[i] )ret[i] = second[i];

	return ret;
}

typedef vector4<float> fvector4;
typedef vector4<double> dvector4;
typedef vector4<int32_t> ivector4;
}
/** @} */
}

template<typename TYPE, size_t SIZE, typename CONTAINER >
::isis::util::FixedVector<TYPE, SIZE, CONTAINER> operator-( const ::isis::util::FixedVector<TYPE, SIZE, CONTAINER>& s )
{
	return s.negate();
}

/// Streaming output for FixedVector
namespace std
{

template<typename charT, typename traits, typename TYPE, size_t SIZE, typename CONTAINER > basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const ::isis::util::FixedVector<TYPE, SIZE, CONTAINER>& s )
{
	s.writeTo( out );
	return out;
}
}
#endif //VECTOR_HPP
