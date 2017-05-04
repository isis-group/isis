/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "bytearray.hpp"
#include <boost/mpl/for_each.hpp>

namespace isis{
namespace data{

ByteArray::ByteArray(const std::shared_ptr<uint8_t>& ptr, size_t length):ValueArray<uint8_t>(ptr,length){}
ByteArray::ByteArray(const ValueArray<uint8_t>& src):ValueArray<uint8_t>(src){}
ByteArray::ByteArray(size_t length):ValueArray<uint8_t>(length){}
ByteArray::ByteArray(uint8_t *const ptr, size_t length):ValueArray<uint8_t>(ptr,length){}


ValueArrayReference ByteArray::atByID( short unsigned int ID, size_t offset, size_t len, bool swap_endianess )
{
	LOG_IF( static_cast<std::shared_ptr<uint8_t>&>( *this ).get() == 0, Debug, error )
			<< "There is no mapped data for this FilePtr - I'm very likely gonna crash soon ..";
	GeneratorMap &map = util::Singletons::get<GeneratorMap, 0>();
	assert( !map.empty() );
	const generator_type gen = map[ID];
	assert( gen );
	return gen( *this, offset, len, swap_endianess );
}
data::ValueArrayBase::ConstReference ByteArray::atByID( unsigned short ID, size_t offset, size_t len, bool swap_endianess)const{
	ValueArrayBase::Reference ref=const_cast<ByteArray*>(this)->atByID(ID,offset,len,swap_endianess);
	const ValueArrayBase &base=*ref;
	return base.castToValueArray<uint8_t>();
}

ByteArray::GeneratorMap::GeneratorMap()
{
	boost::mpl::for_each<util::_internal::types>( proc( this ) );
	assert( !empty() );
}
}
}
