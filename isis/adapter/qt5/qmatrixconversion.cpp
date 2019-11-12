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

// @todo test us
QMatrix Matrix2QMatrix2x2 ( const util::Matrix< qreal, 2, 2 >& matrix )
{
	return QMatrix( matrix[0][0], matrix[0][1], matrix[1][0], matrix[1][1], 0, 0 );
}

util::Matrix< qreal, 2, 2 > QMatrix2Matrix2x2 ( const QMatrix &matrix )
{
	return {matrix.m11(),matrix.m12(),matrix.m21(),matrix.m22()};
}

#if QT_VERSION >= 0x040600

QMatrix4x4 Matrix2QMatrix4x4 ( const util::Matrix4x4< float >& matrix )
{
	QMatrix4x4 ret( std::begin(matrix));
	return ret;
}

util::Matrix4x4< float > QMatrix2Matrix4x4 ( const QMatrix4x4 &matrix )
{
	util::Matrix4x4<float> ret;
	matrix.copyDataTo(std::begin(ret));
	return ret;
}

#endif



}
}
