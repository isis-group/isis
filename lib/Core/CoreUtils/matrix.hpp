/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef ISIS_MATRIX_HPP
#define ISIS_MATRIX_HPP

#include "vector.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/typeof/typeof.hpp>
#include <iomanip>


namespace isis
{
namespace util
{

template < typename TYPE, size_t COLS, size_t ROWS, typename CONTAINER = typename FixedVector<TYPE, ROWS *COLS>::container_type >
class FixedMatrix : public FixedVector<TYPE, ROWS *COLS, CONTAINER>
{
public:
	template<typename A, typename B> struct result_of_mult {
		typedef BOOST_TYPEOF_TPL( A() * B() ) type;
	};
	static const size_t rows = ROWS;
	static const size_t columns = COLS;

	typedef TYPE value_type;

	template<typename TYPE2, typename CONTAINER2>
	void copyFrom( const FixedVector<TYPE2, COLS, CONTAINER2> src[ROWS] ) {
		for( size_t r = 0; r < ROWS; r++ ) {
			const TYPE2 *ptr = &src[r][0];
			std::copy( ptr, ptr + COLS, &elem( 0, r ) );
		}
	}

	FixedMatrix() {}

	template<typename TYPE2>
	FixedMatrix( const TYPE2 src[ROWS *COLS] ): FixedVector<TYPE, ROWS *COLS, CONTAINER>( src ) {}

	template<typename TYPE2, typename CONTAINER2>
	FixedMatrix( const FixedVector<TYPE2, COLS, CONTAINER2> src[ROWS] ) {copyFrom( src );}

	FixedMatrix( const boost::numeric::ublas::matrix<TYPE> &boost_matrix ) throw ( std::logic_error & ) {
		if( boost_matrix.size1() == ROWS && boost_matrix.size2() == COLS ) {
			for( size_t m = 0; m < ROWS; m++ ) {
				for( size_t n = 0; n < COLS; n++ ) {
					this->elem( n, m ) = boost_matrix( m, n );
				}
			}
		} else {
			LOG( Runtime, error ) << "The size of the boost matrix ("
								  << boost_matrix.size1() << ", " << boost_matrix.size2()
								  << ") does not coincide with the size of the isis matrix (" << ROWS << ", " << COLS << ").";
			throw( std::logic_error( "Size mismatch" ) );
		}
	};

	TYPE &elem( size_t column, size_t row ) {return ( *this )[column + row * COLS];}
	const TYPE &elem( size_t column, size_t row )const {return ( *this )[column + row * COLS];}

	boost::numeric::ublas::matrix<TYPE> getBoostMatrix() const {
		boost::numeric::ublas::matrix<TYPE> ret = boost::numeric::ublas::matrix<TYPE>( ROWS, COLS );

		for( size_t m = 0; m < ROWS; m++ ) {
			for( size_t n = 0; n < COLS; n++ ) {
				ret( m, n ) = this->elem( n, m );
			}
		}

		return ret;
	}

	FixedMatrix<TYPE, ROWS, COLS> transpose()const {
		FixedMatrix<TYPE, ROWS, COLS> ret;

		for( size_t m = 0; m < COLS; m++ )
			for( size_t n = 0; n < ROWS; n++ ) {
				ret.elem( n, m ) = this->elem( m, n );
			}

		return ret;
	}

	FixedMatrix<TYPE, COLS, ROWS> inverse( bool &invertible )const throw ( std::logic_error & ) {
		if( COLS != ROWS ) {
			LOG( Runtime, error ) << "Matrix is not a square matrix so is not invertible!";
			throw( std::logic_error( "Matrix is not a square matrix so is not invertible!" ) );
		}

		using namespace boost::numeric::ublas;
		FixedMatrix<TYPE, COLS, ROWS> ret;
		matrix<TYPE> boost_matrix_in = this->getBoostMatrix();
		matrix<TYPE> boost_matrix_inverse = matrix<TYPE>( ROWS, COLS );
		permutation_matrix<TYPE> pm( boost_matrix_in.size1() );
		//check if det is 0 -> singular
		invertible = lu_factorize( boost_matrix_in, pm ) == 0;

		if( invertible ) {
			boost_matrix_inverse.assign( identity_matrix<TYPE>( boost_matrix_in.size1() ) ) ;
			lu_substitute( boost_matrix_in, pm, boost_matrix_inverse );
			return FixedMatrix<TYPE, COLS, ROWS>( boost_matrix_inverse );
		} else {
			LOG( Runtime, error ) << "Matrix is singular. Returning initial matrix.";
			return *this;
		}
	}

	template<typename TYPE2, size_t COLS2, typename CONTAINER2> FixedMatrix<typename result_of_mult<TYPE, TYPE2>::type, COLS2, ROWS>
	dot( const FixedMatrix<TYPE2, COLS2, COLS, CONTAINER2> &right )const {
		// transpose the right, so we can use columns as rows
		typedef typename result_of_mult<TYPE, TYPE2>::type result_type;
		const FixedMatrix<TYPE2, COLS, COLS2, CONTAINER2> rightT = right.transpose();
		const FixedMatrix<TYPE, COLS, ROWS, CONTAINER> &left = *this;
		FixedMatrix<result_type, COLS2, ROWS> ret;

		for( size_t c = 0; c < right.columns; c++ ) { //result has as much columns as right
			const TYPE2 *rstart = &rightT.elem( 0, c );

			for( size_t r = 0; r < left.rows; r++ ) { //result has as much rows as left
				const TYPE *lstart = &left.elem( 0, r ), *lend = lstart + left.columns;
				ret.elem( c, r ) = std::inner_product( lstart, lend, rstart, result_type() );
			}
		}

		return ret;
	}


	template<typename TYPE2, typename CONTAINER2> FixedVector<typename result_of_mult<TYPE, TYPE2>::type, COLS>
	dot( const FixedVector<TYPE2, COLS, CONTAINER2> &right )const {
		const FixedMatrix<TYPE, COLS, ROWS, CONTAINER> &left = *this;
		typedef typename result_of_mult<TYPE, TYPE2>::type result_type;
		FixedVector<result_type, ROWS> ret;
		const TYPE2 *rstart = &right[0];

		for( size_t r = 0; r < rows; r++ ) { //result has as much rows as left
			const TYPE *lstart = &elem( 0, r ), *lend = lstart + left.columns;
			ret[r] = std::inner_product( lstart, lend, rstart, result_type() );
		}

		return ret;
	}

	FixedVector<TYPE, COLS> getRow( size_t rownum )const {
		FixedVector<TYPE, COLS> ret;
		const typename FixedVector<TYPE, ROWS *COLS, CONTAINER>::const_iterator start = FixedVector<TYPE, ROWS * COLS, CONTAINER>::begin() + rownum * COLS;
		const typename FixedVector<TYPE, ROWS *COLS, CONTAINER>::const_iterator end = start + COLS;
		ret.copyFrom( start, end );
		return ret;
	}


};

template<typename TYPE, size_t ELEMS>
class IdentityMatrix : public FixedMatrix<TYPE, ELEMS, ELEMS>
{
public:
	IdentityMatrix( TYPE value = 1 ) {
		for( size_t m = 0; m < ELEMS; m++ ) {
			for( size_t n = 0; n < ELEMS; n++ ) {
				if( m == n ) {
					this->elem( n, m ) = static_cast<TYPE>( value );
				} else {
					this->elem( n, m ) = static_cast<TYPE>( 0 );
				}
			}
		}
	}
};

template<typename TYPE>
class Matrix4x4: public FixedMatrix<TYPE, 4, 4>
{
public:
	Matrix4x4() {};

	Matrix4x4( const FixedMatrix<TYPE, 4, 4> &src ): FixedMatrix<TYPE, 4, 4>( src ) {}

	Matrix4x4( const TYPE src[16] ): FixedMatrix<TYPE, 4, 4>( src ) {}
	template<typename TYPE2> Matrix4x4(
		const FixedVector<TYPE2, 4> &row1,
		const FixedVector<TYPE2, 4> &row2,
		const FixedVector<TYPE2, 4> &row3 = vector4<TYPE2>( 0, 0, 1, 0 ),
		const FixedVector<TYPE2, 4> &row4 = vector4<TYPE2>( 0, 0, 0, 1 )
	) {
        typedef typename FixedVector<TYPE2, 4>::container_type container_type;
		const vector4<TYPE2> src[4] = {row1, row2, row3, row4};
		this->template copyFrom<TYPE2,container_type>( src );
	}
};

template<typename TYPE>
class Matrix3x3: public FixedMatrix<TYPE, 3, 3>
{
public:
	Matrix3x3() {};

	Matrix3x3( const FixedMatrix<TYPE, 3, 3> &src ): FixedMatrix<TYPE, 3, 3>( src ) {}

	Matrix3x3( const TYPE src[9] ): FixedMatrix<TYPE, 3, 3>( src ) {}
	template<typename TYPE2> Matrix3x3(
		const FixedVector<TYPE2, 3> &row1,
		const FixedVector<TYPE2, 3> &row2,
		const FixedVector<TYPE2, 3> &row3 = vector3<TYPE2>( 0, 0, 1 )
	) {
        typedef typename FixedVector<TYPE2, 3>::container_type container_type;
		const vector3<TYPE2> src[3] = {row1, row2, row3};
		this->template copyFrom<TYPE2,container_type>( src );
	}
};

}
}
/// Streaming output for FixedMatrix
namespace std
{

template<typename charT, typename traits, typename TYPE, size_t COLS, size_t ROWS, typename CONTAINER > basic_ostream<charT, traits>&
operator<<( basic_ostream<charT, traits> &out, const ::isis::util::FixedMatrix<TYPE, COLS, ROWS, CONTAINER>& m )
{
	out << "FixedMatrix of size (" << m.columns << " columns, " << m.rows << " rows):" << std::endl;

	for( size_t row = 0; row < ROWS; row++ ) {
		out << "<";

		for( size_t col = 0; col < COLS; col++ ) {
			out << m.elem( col, row );

			if( col < COLS - 1 ) out << "\t";
		}

		out << ">" << std::endl;
	}

	return out;
}
}

#endif // ISIS_MATRIX_HPP
