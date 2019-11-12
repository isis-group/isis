/****************************************************************
 *
 * <Copyright information>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de
 *
 * qmatrixconversion.hpp
 *
 * Description:
 *
 *  Created on: Feb 28, 2012
 *      Author: tuerke
 ******************************************************************/

#ifndef ISIS_QMATRIXCONVERSION_HPP
#define ISIS_QMATRIXCONVERSION_HPP

#include <QMatrix>
#include "../../core/matrix.hpp"
#include <QMatrix4x4>


namespace isis
{
namespace qt5
{

isis::util::Matrix<qreal, 2, 2> QMatrix2Matrix2x2( const QMatrix &matrix );
QMatrix Matrix2QMatrix2x2( const util::Matrix<qreal, 2, 2> &matrix );

isis::util::Matrix4x4<float> QMatrix2Matrix4x4( const QMatrix4x4 &matrix );
QMatrix4x4 Matrix2QMatrix4x4( const util::Matrix4x4<float> &matrix );

template<typename TYPE, unsigned int COLUMN, unsigned int ROW>
isis::util::Matrix<TYPE, COLUMN, ROW> QMatrix2Matrix( const QGenericMatrix<ROW, COLUMN, TYPE> &matrix )
{
	isis::util::Matrix<TYPE, COLUMN, ROW> ret;

	for( unsigned int column = 0; column < COLUMN; column++ ) {
		for( unsigned int row = 0; row < ROW; row++ ) {
			ret.elem( column, row ) = matrix( row, column );
		}
	}

	return ret;
}

template< typename TYPE, unsigned int COLUMN, unsigned int ROW >
QGenericMatrix< ROW, COLUMN, TYPE > Matrix2QMatrix( const util::Matrix<TYPE, COLUMN, ROW> &matrix )
{
	QGenericMatrix<ROW, COLUMN, TYPE> ret;

	for( unsigned int column = 0; column < COLUMN; column++ ) {
		for( unsigned int row = 0; row < ROW; row++ ) {
			ret( row, column ) = matrix( column, row );
		}
	}

	return ret;
}


}
} // end namespace

#endif //ISIS_QMATRIXCONVERSION_HPP
