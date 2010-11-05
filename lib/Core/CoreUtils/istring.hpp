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
#include <ostream>
#include <istream>
#include <boost/lexical_cast.hpp>


namespace isis{namespace util{namespace _internal{

struct ichar_traits: public std::char_traits<char>{
	static bool eq ( const char_type& c1, const char_type& c2);
	static bool lt ( const char_type& c1, const char_type& c2);
	static int compare ( const char_type* s1, const char_type* s2, std::size_t n);
	static const char_type* find ( const char_type* s, std::size_t n, const char_type& a );
};

}

typedef std::basic_string<char,_internal::ichar_traits>    istring;
}
}

namespace std
{
///Streaming output for color using isis::util::write_list
template<typename charT>
basic_ostream<charT, std::char_traits<charT> >& operator<<( basic_ostream<charT, std::char_traits<charT> > &out, const isis::util::istring& s )
{
	return out << std::basic_string<charT, std::char_traits<charT> >(s.begin(),s.end());
}
}

// specialization for boost::lexical_cast
namespace boost{namespace detail{
template<class Source> struct deduce_char_traits<char,isis::util::istring, Source>
{
	typedef isis::util::_internal::ichar_traits type;
};
template<class Traits, class Alloc>
struct deduce_char_traits< char, isis::util::istring, std::basic_string<char,Traits,Alloc> >
{
	typedef isis::util::_internal::ichar_traits type;
};

}}

#endif // UTIL_ISTRING_HPP
