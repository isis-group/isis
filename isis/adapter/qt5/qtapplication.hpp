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

#include "../../util/application.hpp"
#include "../../util/matrix.hpp"
#include "../../data/io_application.hpp"
#include "qdefaultmessageprint.hpp"

namespace isis
{
namespace qt5
{

class QtApplication : public util::Application
{
	std::unique_ptr<QGuiApplication> m_qapp;
public:
	QGuiApplication &getQApplication();
	QtApplication( const char name[] );
	virtual bool init( int &argc, char **argv, bool exitOnError = true );
};

class IOQtApplication : public data::IOApplication
{
	int m_argc; //same as above
	char **m_argv;
	std::unique_ptr<QGuiApplication> m_qapp;
public:
	QGuiApplication &getQApplication();
	IOQtApplication( const char name[], bool have_input = true, bool have_output = true );
	virtual bool init( int &argc, char **argv, bool exitOnError = true );
protected:
	virtual std::shared_ptr<util::MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;

};

}
}

#endif // QT_APPLICATION_HPP
