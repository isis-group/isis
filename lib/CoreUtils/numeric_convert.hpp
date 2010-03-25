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

#ifndef NUMERIC_CONVERT_HPP
#define NUMERIC_CONVERT_HPP

#include <limits>
namespace isis{namespace util{
	template<typename T> class TypePtr;
namespace _internal{
	template<typename SRC,typename DST> void numeric_convert(const TypePtr<SRC> &src,TypePtr<DST> &dst){
		double max,min;
		src.getMinMax(min,max);
		const double range_min=std::numeric_limits<DST>::min();
		const double range_max=std::numeric_limits<DST>::max();
	}
}}}


#endif // NUMERIC_CONVERT_HPP
