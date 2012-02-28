/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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

#include "qtapplication.hpp"

isis::qt4::QtApplication::QtApplication( const char name[] ):
	Application( name )
{}

QApplication &isis::qt4::QtApplication::getQApplication()
{
	LOG_IF( not m_qapp, util::Debug, error ) << "The QApplication was not yet created, you should run init() before using getQApplication.";
	return *m_qapp;
}
bool isis::qt4::QtApplication::init( int &argc, char **argv, bool exitOnError )
{
	if ( m_qapp ) {
		LOG( util::Debug, error ) << "The QApplication allready exists. This should not happen. I'll not touch it";
	} else {
		m_qapp.reset( new QApplication( argc, argv ) );
	}

	return util::Application::init( argc, argv, exitOnError );
}


isis::qt4::IOQtApplication::IOQtApplication( const char name[], bool have_input, bool have_output ):
	IOApplication( name, have_input, have_output )
{}

QApplication &isis::qt4::IOQtApplication::getQApplication()
{
	LOG_IF( not m_qapp, util::Debug, error ) << "The QApplication was not yet created, you should run init() before using getQApplication.";
	return *m_qapp;
}

bool isis::qt4::IOQtApplication::init( int &argc, char **argv, bool exitOnError )
{
	if( m_qapp ) {
		LOG( util::Debug, error ) << "The QApplication allready exists. This should not happen. I'll not touch it";
	} else {
		m_qapp.reset( new QApplication( argc, argv ) );
	}

	return isis::data::IOApplication::init( argc, argv, exitOnError );
}


boost::shared_ptr< isis::util::_internal::MessageHandlerBase > isis::qt4::IOQtApplication::getLogHandler( std::string /*module*/, isis::LogLevel level )const
{
	return boost::shared_ptr< isis::util::_internal::MessageHandlerBase >( level ? new isis::qt4::QDefaultMessagePrint( level ) : 0 );
}


QMatrix isis::qt4::FixedMatrix2QMatrix2x2 ( const isis::util::FixedMatrix< qreal, 2, 2 >& matrix )
{
	QMatrix ret (matrix.elem(0,0), matrix.elem(1,0), matrix.elem(0,1), matrix.elem(1,1), 1, 1 );
	return ret;
}

isis::util::FixedMatrix< qreal, 2, 2 > isis::qt4::QMatrix2FixedMatrix2x2 ( const QMatrix& matrix )
{
	isis::util::FixedMatrix<qreal,2,2> ret;
	ret.elem(0,0) = matrix.m11();
	ret.elem(1,0) = matrix.m12();
	ret.elem(0,1) = matrix.m21();
	ret.elem(1,1) = matrix.m22();
	return ret;
}

QMatrix4x4 isis::qt4::FixedMatrix2QMatrix4x4 ( const isis::util::Matrix4x4< qreal >& matrix )
{
	QMatrix4x4 ret( matrix.elem(0,0), matrix.elem(1,0), matrix.elem(2,0), matrix.elem(3,0),
					matrix.elem(0,1), matrix.elem(1,1), matrix.elem(2,1), matrix.elem(3,1),
					matrix.elem(0,2), matrix.elem(1,2), matrix.elem(2,2), matrix.elem(3,2),
					matrix.elem(0,3), matrix.elem(1,3), matrix.elem(2,3), matrix.elem(3,3)
	);
	return ret;
}

isis::util::Matrix4x4< qreal > isis::qt4::QMatrix2FixedMatrix4x4 ( const QMatrix4x4& matrix )
{
	isis::util::Matrix4x4<qreal> ret;
	for ( unsigned short column = 0; column < 4; column++ ) {
		for ( unsigned short row = 0; row < 4; row++ ) {
			ret.elem(column, row) = matrix(row, column );
		}
	}
	return ret;
}

