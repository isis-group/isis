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

#include "istring.hpp"
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <strings.h>
#endif
#include <algorithm>

namespace isis
{
namespace util
{
namespace _internal
{

std::locale const ichar_traits::loc = std::locale( "C" );

int ichar_traits::compare( const char *s1, const char *s2, size_t n )
{
#ifdef _MSC_VER
	return lstrcmpiA( s1, s2 ); //@todo find some with length parameter
#else
	return strncasecmp( s1, s2, n );
#endif
}

bool ichar_traits::eq( const char &c1, const char &c2 )
{
	return std::tolower( c1, loc ) == std::tolower( c2, loc );
}

bool ichar_traits::lt( const char &c1, const char &c2 )
{
	return std::tolower( c1, loc ) < std::tolower( c2, loc );
}

const char *ichar_traits::find( const char *s, size_t n, const char &a )
{
	const char lowA = std::tolower( a ), upA = std::toupper( a );

	if( lowA == upA ) { // if a has no cases we can do naive search
		return std::find( s, s + n, a );
	} else for( size_t i = 0; i < n; i++, s++ ) {
			if( std::tolower( *s ) == a )
				return s;
		}

	return NULL;
}


}
}
}

namespace boost
{
template<> isis::util::istring lexical_cast< isis::util::istring, std::string >( const std::string &arg ) {return isis::util::istring( arg.begin(), arg.end() );}
template<> std::string lexical_cast< std::string, isis::util::istring >( const isis::util::istring &arg ) {return std::string( arg.begin(), arg.end() );}
}