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
#include <strings.h>

namespace isis{
namespace util{
namespace _internal{

int ichar_traits::compare(const char* s1, const char* s2, size_t n)
{
	return strncasecmp(s1,s2,n);
}

bool ichar_traits::eq(const char& c1, const char& c2)
{
	return std::tolower(c1) == std::tolower(c2);
}

bool ichar_traits::lt(const char& c1, const char& c2)
{
	return std::tolower(c1) < std::tolower(c2);
}

const char* ichar_traits::find(const char* s, size_t n, const char& a)
{
	for(size_t i=0;i<n;i++,s++){
		if(eq(*s,a))
			return s;
	}
	return NULL;
}


}
}
}

