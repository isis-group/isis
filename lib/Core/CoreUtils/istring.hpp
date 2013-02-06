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

#ifndef UTIL_ISTRING_HPP
#define UTIL_ISTRING_HPP

#include <string>
#include <locale>
#include <boost/lexical_cast.hpp>

namespace isis
{
namespace util
{
/// @cond _internal
namespace _internal
{
struct ichar_traits: public std::char_traits<char> {
	static const std::locale loc;
	static bool eq ( const char_type &c1, const char_type &c2 );
	static bool lt ( const char_type &c1, const char_type &c2 );
	static int compare ( const char_type *s1, const char_type *s2, std::size_t n );
	static const char_type *find ( const char_type *s, std::size_t n, const char_type &a );
};
}
/// @endcond _internal

typedef std::basic_string<char, _internal::ichar_traits>    istring;
}
}

namespace std
{
template<typename charT, typename traits>
basic_ostream<charT, traits>& operator<<( basic_ostream<charT, traits> &out, const isis::util::istring &s )
{
	return out << s.c_str();
}
}

namespace boost
{
// specialization for boost::lexical_cast to differ std::string and util::istring
template<> isis::util::istring lexical_cast<isis::util::istring, std::string>        ( const std::string         &arg );
// specialization for boost::lexical_cast to differ std::string and util::istring
template<> std::string         lexical_cast<std::string,         isis::util::istring>( const isis::util::istring &arg );
}
#endif // UTIL_ISTRING_HPP
