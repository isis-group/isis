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

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "progparameter.hpp"


namespace isis
{
namespace util
{

class Application
{
	std::string m_name;
	static const LogLevel LLMap[];
protected:
	virtual boost::shared_ptr<_internal::MessageHandlerBase> getLogHandler( std::string module, isis::LogLevel level )const;
public:
	ParameterMap parameters;
	Application( const char name[] );
	virtual ~Application();
	virtual bool init( int argc, char** argv, bool exitOnError = true );
	virtual void printHelp()const;
	template<typename MODULE> void setLog( LogLevel level ) {
		if ( !MODULE::use );
		else _internal::Log<MODULE>::setHandler( getLogHandler( MODULE::name(), level ) );
	}
};


}
}
#endif // APPLICATION_HPP
