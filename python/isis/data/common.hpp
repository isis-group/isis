/*
 * common.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef PYTHON_COMMON_DATA_HPP
#define PYTHON_COMMON_DATA_HPP

#include "CoreUtils/log.hpp"
#include "DataStorage/chunk.hpp"
#include "DataStorage/valuearray.hpp"
#include <boost/python.hpp>

using namespace isis::data;
namespace isis
{
namespace python
{
namespace data
{
namespace _internal
{
using namespace boost::python;

struct VoxelOp {

	static boost::python::api::object getVoxelAsPyObject( const isis::data::Chunk &ch, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		switch( ch.getTypeID() ) {
		case ValueArray<int8_t>::staticID:
			return api::object( ch.voxel<int8_t>( first, second, third, fourth ) );
			break;
		case ValueArray<uint8_t>::staticID:
			return api::object( ch.voxel<uint8_t>( first, second, third, fourth ) );
			break;
		case ValueArray<int16_t>::staticID:
			return api::object( ch.voxel<int16_t>( first, second, third, fourth ) );
			break;
		case ValueArray<uint16_t>::staticID:
			return api::object( ch.voxel<uint16_t>( first, second, third, fourth ) );
			break;
		case ValueArray<int32_t>::staticID:
			return api::object( ch.voxel<int32_t>( first, second, third, fourth ) );
			break;
		case ValueArray<uint32_t>::staticID:
			return api::object( ch.voxel<uint32_t>( first, second, third, fourth ) );
			break;
		case ValueArray<int64_t>::staticID:
			return api::object( ch.voxel<int64_t>( first, second, third, fourth ) );
			break;
		case ValueArray<float>::staticID:
			return api::object( ch.voxel<float>( first, second, third, fourth ) );
			break;
		case ValueArray<double>::staticID:
			return api::object( ch.voxel<double>( first, second, third, fourth ) );
			break;
		}

		return api::object( 0 ); // prevent warnings
	}

	static bool setVoxelAsPyObject( isis::data::Chunk &ch, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const boost::python::api::object &value ) {
		switch( ch.getTypeID() ) {
		case ValueArray<int8_t>::staticID:
			ch.voxel<int8_t>( first, second, third, fourth ) = extract<int8_t>( value );
			return true;
			break;
		case ValueArray<uint8_t>::staticID:
			ch.voxel<uint8_t>( first, second, third, fourth ) = extract<uint8_t>( value );
			return true;
			break;
		case ValueArray<int16_t>::staticID:
			ch.voxel<int16_t>( first, second, third, fourth ) = extract<int16_t>( value );
			return true;
			break;
		case ValueArray<uint16_t>::staticID:
			ch.voxel<uint16_t>( first, second, third, fourth ) = extract<uint16_t>( value );
			return true;
			break;
		case ValueArray<int32_t>::staticID:
			ch.voxel<int32_t>( first, second, third, fourth ) = extract<int32_t>( value );
			return true;
			break;
		case ValueArray<uint32_t>::staticID:
			ch.voxel<uint32_t>( first, second, third, fourth ) = extract<uint32_t>( value );
			return true;
			break;
		case ValueArray<int64_t>::staticID:
			ch.voxel<int64_t>( first, second, third, fourth ) = extract<int64_t>( value );
			return true;
			break;
		case ValueArray<uint64_t>::staticID:
			ch.voxel<uint64_t>( first, second, third, fourth ) = extract<uint64_t>( value );
			return true;
			break;
		case ValueArray<float>::staticID:
			ch.voxel<float>( first, second, third, fourth ) = extract<float>( value );
			return true;
			break;
		case ValueArray<double>::staticID:
			ch.voxel<double>( first, second, third, fourth ) = extract<double>( value );
			return true;
			break;
		}

		return false;
	}

};

template <typename TYPE>
list stdIter2PyList( const TYPE &stdList )
{
	list retList;
	BOOST_FOREACH( const typename TYPE::const_reference listItem, stdList ) {
		retList.append( listItem );
	}
	return retList;
}

template <typename TYPE>
std::list<TYPE> pyList2StdList( const list &pyList )
{
	std::list<TYPE> retList;

	for( unsigned short i = 0; i < boost::python::len( pyList ); i++ ) {
		retList.push_back( extract<TYPE>( pyList[i] ) );
	}

	return retList;
}

}
}
} //namespace python

} //namespace isis
#endif /* PYTHON_COMMON_HPP_ */


