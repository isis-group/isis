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
#include "common.hpp"

namespace isis{
namespace qt4 {

QtApplication::QtApplication( const char name[] ):
		Application( name )
{
	util::Selection dbg_levels( "error,warning,info,verbose_info" );
	dbg_levels.set( "warning" );
	parameters["dQt4"] = dbg_levels;
	parameters["dQt4"].setDescription( "Debugging level for the qt4 adapter module" );
}

QApplication& QtApplication::getQApplication()
{
	LOG_IF( not m_qapp, Debug, error ) << "The QApplication was not yet created, you should run init() before using getQApplication.";
	return *m_qapp;
}
bool QtApplication::init( int argc, char** argv, bool exitOnError )
{
	bool ret=util::Application::init( argc, argv, exitOnError );// run init of the base first - will fill parameters
	setLog<Debug>( LLMap[parameters["dQt4"]] );
	setLog<Runtime>( LLMap[parameters["dQt4"]] );

	if ( m_qapp ) {
		LOG( Debug, error ) << "The QApplication allready exists. This should not happen. I'll not touch it";
	} else {
		m_argc = argc; // create local copies
		m_argv = argv;
		m_qapp.reset( new QApplication( m_argc, m_argv ) );
	}
	return ret;
}


}}
