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

#include <iomanip>


namespace isis
{
namespace util
{
	
/// Row-Major Matrix (is made of ROWS arrays with the width COLS)
template<typename TYPE, size_t COLS, size_t ROWS> using Matrix = std::array<std::array<TYPE, COLS>,ROWS>; 
template<typename TYPE> using Matrix3x3 = Matrix<TYPE,3,3>;
template<typename TYPE> using Matrix4x4 = Matrix<TYPE,4,4>;

/// \return the transposed matrix of mat
template<typename TYPE, size_t COLS, size_t ROWS> Matrix<TYPE, ROWS, COLS> 
transpose(const Matrix<TYPE, COLS, ROWS> &mat){
	Matrix<TYPE, ROWS, COLS> ret;

	for( size_t m = 0; m < COLS; m++ )
		for( size_t n = 0; n < ROWS; n++ ) {
			ret[n][m] = mat[m][n];
		}

	return ret;
}

/**
 * Fuzzy comparison for matrices.
 * Does util::fuzzyEqual for the associated elements of the two vectors.
 * @param other the "other" vector to compare to
 * @param scale scaling factor forwarded to util::fuzzyEqual
 * \returns true if util::fuzzyEqual for all elements
 */
template<typename TYPE1, typename TYPE2, size_t COLS, size_t ROWS> bool
fuzzyEqualM( const Matrix<TYPE1, COLS, ROWS> &first, const Matrix<TYPE2, COLS, ROWS> &second, unsigned short scale = 10 ){
	auto b = std::begin(second);

	for ( const std::array<TYPE1,COLS> &a : first ) {
		if ( ! fuzzyEqualV( a, *(b++), scale ) )
			return false;
	}

	return true;
}

template<typename TYPE, size_t COLS, size_t ROWS=COLS> constexpr Matrix<TYPE, ROWS, COLS> 
identityMatrix( TYPE value = 1 ){
	Matrix<TYPE, ROWS, COLS> ret{0,0,0,0};
	for( size_t m = 0; m < COLS; m++ ) {
		for( size_t n = 0; n < ROWS; n++ ) {
			ret[n][m] = (m == n)?value:TYPE();
		}
	}
	return ret;
}

	
}

template<typename TYPE1, typename TYPE2,size_t COLS, size_t ROWS, size_t COLS2> util::Matrix<decltype(TYPE1()*TYPE2()), COLS2, ROWS>
operator*( const util::Matrix<TYPE1, COLS, ROWS> &left, const util::Matrix<TYPE2, COLS2, COLS> &right ){
	// transpose the right, so we can use columns as rows
	typedef decltype(TYPE1()*TYPE2()) result_type;
	const util::Matrix<TYPE2, COLS, COLS2> rightT = util::transpose(right);
	util::Matrix<result_type, COLS2, ROWS> ret;

	for( size_t c = 0; c < COLS2; c++ ) { //result has as much columns as right
		const std::array<TYPE2, COLS> &rcol = rightT[c]; //columns of right are rows of rightT 
		for( size_t r = 0; r < ROWS; r++ ) { //result has as much rows as left
			const std::array<TYPE1, COLS> &lrow = left[r];
			ret[r][c] = std::inner_product( std::begin(lrow), std::end(lrow), std::begin(rcol), result_type() );
		}
	}

	return ret;
}

template<typename TYPE1, typename TYPE2,size_t COLS, size_t ROWS> std::array<decltype(TYPE1()*TYPE2()), ROWS> 
operator*( const util::Matrix<TYPE1, COLS, ROWS> &left, const std::array<TYPE2, COLS> &right ){
	typedef decltype(TYPE1()*TYPE2()) result_type;
	std::array<result_type, ROWS> ret;

	for( size_t r = 0; r < ROWS; r++ ) { //result has as much rows as left
		const std::array<TYPE1, COLS> &lrow = left[r];
		ret[r] = std::inner_product( std::begin(lrow), std::end(lrow), std::begin(right), result_type() );
	}

	return ret;
}
}

#endif // ISIS_MATRIX_HPP
