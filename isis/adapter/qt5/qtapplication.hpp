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

#include <QGuiApplication>
#include <QMatrix4x4>

#include "../../core/application.hpp"
#include "../../core/matrix.hpp"
#include "../../core/io_application.hpp"
#include "qdefaultmessageprint.hpp"
#include <QApplication>

namespace isis
{
namespace qt5
{

class QtApplication : public util::Application
{
	std::unique_ptr<QApplication> m_qapp;
public:
	QApplication &getQApplication();
	QtApplication( const char name[] );
	virtual bool init( int &argc, char **argv, bool exitOnError = true );
	virtual std::shared_ptr<util::MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;
	int exec();
};

class IOQtApplication : public data::IOApplication
{
	int m_argc; //same as above
	char **m_argv;
	std::unique_ptr<QApplication> m_qapp;
public:
	QApplication &getQApplication();
	IOQtApplication( const char name[], bool have_input = true, bool have_output = true );
	virtual bool init( int &argc, char **argv, bool exitOnError = true );
	virtual std::shared_ptr<util::MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;
	int exec();
};

}
}

#endif // QT_APPLICATION_HPP
