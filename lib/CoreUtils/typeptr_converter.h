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

#ifndef TYPEPTR_CONVERTER_H
#define TYPEPTR_CONVERTER_H

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include "log.hpp"

namespace isis
{
namespace util
{
namespace _internal
{

class TypeBase;
class TypePtrBase;
class TypePtrConverterBase
{
public:
	virtual void convert( const TypePtrBase& src, TypePtrBase &dst, const TypeBase &min, const TypeBase &max )const;
	virtual void generate( const boost::scoped_ptr<TypePtrBase>& src, boost::scoped_ptr<TypePtrBase>& dst, const TypeBase &min, const TypeBase &max )const = 0;
	static boost::shared_ptr<const TypePtrConverterBase> create() {return boost::shared_ptr<const TypePtrConverterBase>();}
public:
	virtual ~TypePtrConverterBase() {}
};

class TypePtrConverterMap : public std::map< int , std::map<int, boost::shared_ptr<const TypePtrConverterBase> > >
{
public:
	TypePtrConverterMap();
};

}
}
}

#endif // TYPEPTR_CONVERTER_H
