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

#ifndef QT_APPLICATION_HPP
#define QT_APPLICATION_HPP

#include <QApplication>
#include <QMatrix>
#include <QMatrix4x4>

#include <CoreUtils/application.hpp>
#include <CoreUtils/matrix.hpp>
#include <DataStorage/io_application.hpp>
#include <boost/scoped_ptr.hpp>
#include "qdefaultmessageprint.hpp"

namespace isis
{
namespace qt4
{

class QtApplication : public util::Application
{
	boost::scoped_ptr<QApplication> m_qapp;
public:
	QApplication &getQApplication();
	QtApplication( const char name[] );
	/// see http://developer.qt.nokia.com/doc/qt-4.8/qapplication.html#QApplication
	virtual bool init( int &argc, char **argv, bool exitOnError = true );
};

class IOQtApplication : public data::IOApplication
{
	int m_argc; //same as above
	char **m_argv;
	boost::scoped_ptr<QApplication> m_qapp;
public:
	QApplication &getQApplication();
	IOQtApplication( const char name[], bool have_input = true, bool have_output = true );
	/// see http://developer.qt.nokia.com/doc/qt-4.8/qapplication.html#QApplication
	virtual bool init( int &argc, char **argv, bool exitOnError = true );
protected:
	virtual boost::shared_ptr<util::_internal::MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;

};

}
}

#endif // QT_APPLICATION_HPP
