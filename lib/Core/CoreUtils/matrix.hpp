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


#ifndef MATRIX_HPP
#define MATRIX_HPP

#include "vector.hpp"


namespace isis
{
namespace util
{

template < typename TYPE, size_t COLS, size_t ROWS, typename CONTAINER = typename FixedVector<TYPE, ROWS *COLS>::container_type >
class FixedMatrix : public FixedVector<TYPE, ROWS *COLS, CONTAINER>
{
public:
	static const size_t rows = ROWS;
	static const size_t columns = COLS;

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


	TYPE &elem( size_t column, size_t row ) {return ( *this )[column + row * COLS];}
	const TYPE &elem( size_t column, size_t row )const {return ( *this )[column + row * COLS];}

	FixedMatrix<TYPE, ROWS, COLS> transpose()const {
		FixedMatrix<TYPE, ROWS, COLS> ret;

		for( size_t m = 0; m < COLS; m++ )
			for( size_t n = 0; n < ROWS; n++ ) {
				ret.elem( n, m ) = this->elem( m, n );
			}

		return ret;
	}
	template<typename TYPE2, size_t COLS2, typename CONTAINER2> FixedMatrix<TYPE2, COLS2, ROWS, CONTAINER2>
	dot( const FixedMatrix<TYPE2, COLS2, COLS, CONTAINER2> &right )const {
		// transpose the right, so we can use columns as rows
		const FixedMatrix<TYPE2, COLS, COLS2, CONTAINER2> rightT = right.transpose();
		const FixedMatrix<TYPE, COLS, ROWS, CONTAINER> &left = *this;
		FixedMatrix<TYPE2, COLS2, ROWS, CONTAINER2> ret;

		for( size_t c = 0; c < right.columns; c++ ) { //result has as much columns as right
			const TYPE2 *rstart = &rightT.elem( 0, c );

			for( size_t r = 0; r < left.rows; r++ ) { //result has as much rows as left
				const TYPE *lstart = &left.elem( 0, r ), *lend = lstart + left.columns;
				ret.elem( c, r ) = std::inner_product( lstart, lend, rstart, TYPE2() );
			}
		}

		return ret;
	}

	template<typename TYPE2, typename CONTAINER2> FixedVector<TYPE2, COLS, CONTAINER2>
	dot( const FixedVector<TYPE2, COLS, CONTAINER2> &right )const {
		const FixedMatrix<TYPE, COLS, ROWS, CONTAINER> &left = *this;
		FixedVector<TYPE2, ROWS, CONTAINER2> ret;
		const TYPE2 *rstart = &right[0];

		for( size_t r = 0; r < rows; r++ ) { //result has as much rows as left
			const TYPE *lstart = &elem( 0, r ), *lend = lstart + left.columns;
			ret[r] = std::inner_product( lstart, lend, rstart, TYPE2() );
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

template<typename TYPE>
class Matrix4x4: public FixedMatrix<TYPE, 4, 4>
{
public:
	Matrix4x4( const FixedMatrix<TYPE, 4, 4> &src ): FixedMatrix<TYPE, 4, 4>( src ) {}

	Matrix4x4( const TYPE src[16] ): FixedMatrix<TYPE, 4, 4>( src ) {}
	template<typename TYPE2> Matrix4x4(
		const FixedVector<TYPE2, 4> &row1,
		const FixedVector<TYPE2, 4> &row2,
		const FixedVector<TYPE2, 4> &row3 = vector4<TYPE2>( 0, 0, 1, 0 ),
		const FixedVector<TYPE2, 4> &row4 = vector4<TYPE2>( 0, 0, 0, 1 )
	) {
		const vector4<TYPE2> src[4] = {row1, row2, row3, row4};
		FixedMatrix<TYPE, 4, 4>::copyFrom( src );
	}
};

}
}


#endif // MATRIX_HPP
