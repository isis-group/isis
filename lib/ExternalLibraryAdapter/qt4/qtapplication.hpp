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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include "CoreUtils/application.hpp"
#include <boost/scoped_ptr.hpp>

namespace isis
{
namespace qt4
{

class QtApplication : public util::Application
{
	int m_argc; //we need local copies here, so we can give references to QApplication
	char** m_argv;
	boost::scoped_ptr<QApplication> m_qapp;
public:
	QApplication &getQApplication();
	QtApplication( const char name[] );
	virtual bool init( int argc, char** argv, bool exitOnError = true );
};
}
}

#endif // APPLICATION_H
