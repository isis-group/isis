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
 * qmatrixconversion.cpp
 *
 * Description:
 *
 *  Created on: Feb 28, 2012
 *      Author: tuerke
 ******************************************************************/

#include "qmatrixconversion.hpp"

namespace isis
{
namespace qt5
{


QMatrix FixedMatrix2QMatrix2x2 ( const util::FixedMatrix< qreal, 2, 2 >& matrix )
{
	QMatrix ret ( matrix.elem( 0, 0 ), matrix.elem( 1, 0 ), matrix.elem( 0, 1 ), matrix.elem( 1, 1 ), 1, 1 );
	return ret;
}

util::FixedMatrix< qreal, 2, 2 > QMatrix2FixedMatrix2x2 ( const QMatrix &matrix )
{
	util::FixedMatrix<qreal, 2, 2> ret;
	ret.elem( 0, 0 ) = matrix.m11();
	ret.elem( 1, 0 ) = matrix.m12();
	ret.elem( 0, 1 ) = matrix.m21();
	ret.elem( 1, 1 ) = matrix.m22();
	return ret;
}

#if QT_VERSION >= 0x040600

QMatrix4x4 FixedMatrix2QMatrix4x4 ( const util::Matrix4x4< qreal >& matrix )
{
	QMatrix4x4 ret( matrix.elem( 0, 0 ), matrix.elem( 1, 0 ), matrix.elem( 2, 0 ), matrix.elem( 3, 0 ),
					matrix.elem( 0, 1 ), matrix.elem( 1, 1 ), matrix.elem( 2, 1 ), matrix.elem( 3, 1 ),
					matrix.elem( 0, 2 ), matrix.elem( 1, 2 ), matrix.elem( 2, 2 ), matrix.elem( 3, 2 ),
					matrix.elem( 0, 3 ), matrix.elem( 1, 3 ), matrix.elem( 2, 3 ), matrix.elem( 3, 3 )
				  );
	return ret;
}

util::Matrix4x4< qreal > QMatrix2FixedMatrix4x4 ( const QMatrix4x4 &matrix )
{
	util::Matrix4x4<qreal> ret;

	for ( unsigned short column = 0; column < 4; column++ ) {
		for ( unsigned short row = 0; row < 4; row++ ) {
			ret.elem( column, row ) = matrix( row, column );
		}
	}

	return ret;
}

#endif



}
}
