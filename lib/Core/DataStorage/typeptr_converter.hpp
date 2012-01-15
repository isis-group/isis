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
#include <map>
#include "../CoreUtils/type_base.hpp"

namespace isis
{
namespace data
{
enum autoscaleOption {noscale, autoscale, noupscale, upscale};
typedef std::pair<util::ValueReference, util::ValueReference> scaling_pair;
class ValuePtrBase;

#ifndef WIN32
#pragma GCC visibility push(hidden)
#endif
namespace _internal
{
class ValuePtrConverterBase
{
public:
	virtual void convert( const ValuePtrBase &src, ValuePtrBase &dst, const scaling_pair &scaling )const;
	virtual void generate( const ValuePtrBase &src, boost::scoped_ptr<ValuePtrBase>& dst, const scaling_pair &scaling )const = 0;
	/// Create a ValuePtr based on the ID - if len==0 a pointer to NULL is created
	virtual void create( boost::scoped_ptr<ValuePtrBase>& dst, size_t len )const = 0;
	virtual scaling_pair getScaling( const util::ValueBase &min, const util::ValueBase &max, autoscaleOption scaleopt = autoscale )const;
	static boost::shared_ptr<const ValuePtrConverterBase> get() {return boost::shared_ptr<const ValuePtrConverterBase>();}
	virtual ~ValuePtrConverterBase() {}
};

class ValuePtrConverterMap : public std::map< int , std::map<int, boost::shared_ptr<const ValuePtrConverterBase> > >
{
public:
	ValuePtrConverterMap();
};

}
#ifndef WIN32
#pragma GCC visibility pop
#endif
}
}

#endif // TYPEPTR_CONVERTER_H
