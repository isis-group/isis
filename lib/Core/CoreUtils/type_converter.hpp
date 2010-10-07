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

#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/numeric/conversion/converter.hpp>
#include "log.hpp"


namespace isis
{
namespace util
{
namespace _internal
{

class TypeBase;
class TypeConverterBase
{
public:
	virtual boost::numeric::range_check_result convert( const TypeBase &src, TypeBase &dst )const = 0;
	virtual boost::numeric::range_check_result generate( const TypeBase &src, boost::scoped_ptr<TypeBase>& dst )const = 0;
	static boost::shared_ptr<const TypeConverterBase> create() {return boost::shared_ptr<const TypeConverterBase>();}
public:
	virtual ~TypeConverterBase() {}
};

class TypeConverterMap : public std::map< int , std::map<int, boost::shared_ptr<const TypeConverterBase> > >
{
public:
	TypeConverterMap();
};

}
}
}
#endif // CONVERTER_HPP
