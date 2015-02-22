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

#include <memory>
#include <map>
#include "../CoreUtils/value_base.hpp"

/// @cond _internal

namespace isis
{
namespace data
{
enum autoscaleOption {noscale, autoscale, noupscale, upscale};
typedef std::pair<util::ValueReference, util::ValueReference> scaling_pair;
class ValueArrayBase;

API_EXCLUDE_BEGIN;
namespace _internal
{
class ValueArrayConverterBase
{
public:
	virtual void convert( const ValueArrayBase &src, ValueArrayBase &dst, const scaling_pair &scaling )const;
	virtual void generate( const ValueArrayBase &src, std::unique_ptr<ValueArrayBase>& dst, const scaling_pair &scaling )const = 0;
	/// Create a ValueArray based on the ID - if len==0 a pointer to NULL is created
	virtual void create( std::unique_ptr<ValueArrayBase>& dst, size_t len )const = 0;
	virtual scaling_pair getScaling( const util::ValueBase &min, const util::ValueBase &max, autoscaleOption scaleopt = autoscale )const;
	static std::shared_ptr<const ValueArrayConverterBase> get() {return std::shared_ptr<const ValueArrayConverterBase>();}
	virtual ~ValueArrayConverterBase() {}
};

class ValueArrayConverterMap : public std::map< int , std::map<int, std::shared_ptr<const ValueArrayConverterBase> > >
{
public:
	ValueArrayConverterMap();
};

}
API_EXCLUDE_END;
}
}

/// @endcond _internal
#endif // TYPEPTR_CONVERTER_H
