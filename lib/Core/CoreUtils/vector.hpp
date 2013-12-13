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

#include <boost/numeric/ublas/vector.hpp>
#include <boost/mpl/and.hpp>

namespace isis
{
namespace util
{
/// @cond _internal
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
struct VectorClass{};//empty base class to recognize vectors
}
/// @endcond _internal

template < typename TYPE, size_t SIZE, typename CONTAINER = _internal::array<TYPE, SIZE> >
class FixedVector:protected CONTAINER,public boost::arithmetic<FixedVector<TYPE, SIZE, CONTAINER>, TYPE>,public _internal::VectorClass
{
public:
	typedef typename CONTAINER::iterator iterator;
	typedef typename CONTAINER::const_iterator const_iterator;
	typedef FixedVector<TYPE, SIZE, CONTAINER> this_class;
	typedef CONTAINER container_type;
	typedef TYPE value_type;
protected:
	/// Generic operations
	template<typename OP> this_class& binaryOp ( const TYPE &src ){
		const OP op = OP();
		for ( iterator i = CONTAINER::begin(); i != CONTAINER::end(); i++)
			*i = op( *i, src );
		return *this;
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
	template<typename TYPE2, typename CONTAINER2> bool operator==( const FixedVector<TYPE2, SIZE, CONTAINER2> &src )const {return std::equal( CONTAINER::begin(), CONTAINER::end(), src.begin() );}
	template<typename TYPE2, typename CONTAINER2> bool operator!=( const FixedVector<TYPE2, SIZE, CONTAINER2> &src )const {return !this->operator==(src);}
	/**
	 * Fuzzy comparison for vectors.
	 * Does util::fuzzyEqual for the associated elements of the two vectors.
	 * @param other the "other" vector to compare to
	 * @param scale scaling factor forwarded to util::fuzzyEqual
	 * \returns true if util::fuzzyEqual for all elements
	 */
	bool fuzzyEqual( const this_class &other, unsigned short scale = 10 )const {
		const_iterator b = other.begin();

		for ( const_iterator a = CONTAINER::begin(); a != CONTAINER::end(); ++a, ++b ) {
			if ( ! util::fuzzyEqual( *a, *b, scale ) )
				return false;
		}

		return true;
	}

	////////////////////////////////////////////////////////////////////////////////////
	// Arithmetic operations for scalars (other are defined further down outside of the class)
	////////////////////////////////////////////////////////////////////////////////////
	this_class& operator-=( const TYPE &src ) {return binaryOp<std::minus<TYPE>      >( src );}
	this_class& operator+=( const TYPE &src ) {return binaryOp<std::plus<TYPE>       >( src );}
	this_class& operator*=( const TYPE &src ) {return binaryOp<std::multiplies<TYPE> >( src );}
	this_class& operator/=( const TYPE &src ) {return binaryOp<std::divides<TYPE>    >( src );}

	///\returns a negated copy
	const this_class negate()const {
		this_class ret=*this;
		std::transform( ret.begin(), ret.end(), ret.begin(), std::negate<TYPE>() );
		return ret;
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
	TYPE sum() const {
		TYPE ret = 0;

		for ( const_iterator i = CONTAINER::begin(); i != CONTAINER::end(); i++ )
			ret += *i;

		return ret;
	}

	boost::numeric::ublas::vector<TYPE> getBoostVector() const {
		boost::numeric::ublas::vector<TYPE> ret = boost::numeric::ublas::vector<TYPE>( SIZE );

		for( size_t i = 0; i < SIZE; i++ ) {
			ret( i ) = operator[]( i );
		}

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

	iterator begin() {return CONTAINER::begin();}
	iterator end() {return CONTAINER::end();}
	const_iterator begin()const {return CONTAINER::begin();}
	const_iterator end()const {return CONTAINER::end();}
};

template<typename TYPE>
class vector4 : public FixedVector<TYPE, 4>
{
public:
	vector4() {}
	template<typename TYPE2, typename CONTAINER2> vector4( const FixedVector<TYPE2, 4, CONTAINER2> &src ) : FixedVector< TYPE, 4> ( src ) {}
	vector4( const TYPE src[4] ): FixedVector< TYPE, 4>( src ) {}
	vector4( TYPE first, TYPE second, TYPE third = 0, TYPE fourth = 0 ) {
		this->operator[]( 3 ) = fourth;
		this->operator[]( 2 ) = third;
		this->operator[]( 1 ) = second;
		this->operator[]( 0 ) = first;
	}
};

template<typename TYPE>
class vector3 : public FixedVector<TYPE, 3>
{
public:
	vector3() {}
	template<typename TYPE2, typename CONTAINER2> vector3( const FixedVector<TYPE2, 3, CONTAINER2> &src ) : FixedVector< TYPE, 3> ( src ) {}
	vector3( const TYPE src[3] ): FixedVector< TYPE, 3>( src ) {}
	vector3( TYPE first, TYPE second, TYPE third = 0 ) {
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
typedef vector3<float> fvector3;
typedef vector3<double> dvector3;
typedef vector4<int32_t> ivector4;

API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal {
	using boost::mpl::and_;
	using boost::is_base_of;
	template<typename VEC1,typename VEC2> struct valid_vecs : and_<is_base_of<VectorClass,VEC1>,is_base_of<VectorClass,VEC2> >{};
	template<typename OP,typename VEC1,typename VEC2> VEC1& binary_vec_op ( VEC1& rhs,const VEC2 &lhs ){
		std::transform( rhs.begin(), rhs.end(), lhs.begin(), rhs.begin(), OP() );
		return rhs;
	}
	
}
/// @endcond _internal
API_EXCLUDE_END;
}
}

template<typename TYPE, size_t SIZE, typename CONTAINER >
::isis::util::FixedVector<TYPE, SIZE, CONTAINER> operator-( const ::isis::util::FixedVector<TYPE, SIZE, CONTAINER>& s )
{
	return s.negate();
}



template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type &operator+=( VEC1& rhs,const VEC2 &lhs ) {return isis::util::_internal::binary_vec_op<std::plus<typename VEC1::value_type> >(rhs,lhs);}
template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type &operator-=( VEC1& rhs,const VEC2 &lhs ) {return isis::util::_internal::binary_vec_op<std::minus<typename VEC1::value_type> >(rhs,lhs);}
template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type &operator*=( VEC1& rhs,const VEC2 &lhs ) {return isis::util::_internal::binary_vec_op<std::multiplies<typename VEC1::value_type> >(rhs,lhs);}
template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type &operator/=( VEC1& rhs,const VEC2 &lhs ) {return isis::util::_internal::binary_vec_op<std::divides<typename VEC1::value_type> >(rhs,lhs);}

template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type operator+( VEC1 rhs,const VEC2 &lhs ) {return rhs+=lhs;}
template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type operator-( VEC1 rhs,const VEC2 &lhs ) {return rhs-=lhs;}
template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type operator*( VEC1 rhs,const VEC2 &lhs ) {return rhs*=lhs;}
template<typename VEC1, typename VEC2> typename boost::enable_if<isis::util::_internal::valid_vecs<VEC1,VEC2>, VEC1>::type operator/( VEC1 rhs,const VEC2 &lhs ) {return rhs/=lhs;}

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
