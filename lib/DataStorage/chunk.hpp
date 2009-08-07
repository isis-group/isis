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

#ifndef CHUNK_H
#define CHUNK_H

#include "CoreUtils/type.hpp"

namespace iData{

class Chunk : ::iUtil::_internal::TypeBase
{
public:
	bool is(const std::type_info & t);
	std::string toString(bool labeled=false);
	std::string typeName();
	unsigned short typeID();
};
}
#endif // CHUNK_H
