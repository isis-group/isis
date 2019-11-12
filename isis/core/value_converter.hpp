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

#include <memory>
#include <boost/numeric/conversion/converter.hpp>
#include "log.hpp"
#include "../config.hpp"

/// @cond _internal

namespace isis
{
namespace util
{
class ValueBase;
namespace _internal
{
class ValueConverterBase
{
public:
	virtual boost::numeric::range_check_result convert( const ValueBase &src, ValueBase &dst )const = 0;
	virtual void create( std::unique_ptr<ValueBase>& dst )const = 0;
	virtual boost::numeric::range_check_result generate( const ValueBase &src, std::unique_ptr<ValueBase>& dst )const = 0;
	static std::shared_ptr<const ValueConverterBase> get() {return std::shared_ptr<const ValueConverterBase>();}
public:
	virtual ~ValueConverterBase() {}
};

API_EXCLUDE_BEGIN;
class ValueConverterMap : public std::map< int , std::map<int, std::shared_ptr<const ValueConverterBase> > >
{
public:
	ValueConverterMap();
};

}
API_EXCLUDE_END;
}
}
/// @endcond _internal
#endif // CONVERTER_HPP
