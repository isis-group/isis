/*
 * _chunk.hpp
 *
 *  Created on: Oct 21, 2010
 *      Author: tuerke
 */

#ifndef CHUNK_HPP_
#define CHUNK_HPP_

#include "DataStorage/chunk.hpp"

using namespace isis::data;

namespace isis
{
namespace python
{
namespace data {
	
class _Chunk : public Chunk, boost::python::wrapper<Chunk>
{
public:
	//  _Chunk ( PyObject *p ) : self( p ) {}
	_Chunk ( PyObject *p, const Chunk &base ) : Chunk( base ), self( p ) {}

	float _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		switch( getTypeID() ) {
		case ValuePtr<int8_t>::staticID:
			return Chunk::voxel<int8_t>( first, second, third, fourth );
			break;
		case ValuePtr<uint8_t>::staticID:
			return Chunk::voxel<uint8_t>( first, second, third, fourth );
			break;
		case ValuePtr<int16_t>::staticID:
			return Chunk::voxel<int16_t>( first, second, third, fourth );
			break;
		case ValuePtr<uint16_t>::staticID:
			return Chunk::voxel<uint16_t>( first, second, third, fourth );
			break;
		case ValuePtr<int32_t>::staticID:
			return Chunk::voxel<int32_t>( first, second, third, fourth );
			break;
		case ValuePtr<uint32_t>::staticID:
			return Chunk::voxel<uint32_t>( first, second, third, fourth );
			break;
		case ValuePtr<float>::staticID:
			return Chunk::voxel<float>( first, second, third, fourth );
			break;
		case ValuePtr<double>::staticID:
			return Chunk::voxel<double>( first, second, third, fourth );
			break;
		default:
			return 0;
		}
	}

	float _voxel( const isis::util::ivector4 &coord ) {
		return _voxel( coord[0], coord[1], coord[2], coord[3] );
	}

	bool _setVoxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const float &value ) {

		switch( getTypeID() ) {
		case ValuePtr<int8_t>::staticID:
			Chunk::voxel<int8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<uint8_t>::staticID:
			Chunk::voxel<uint8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<int16_t>::staticID:
			Chunk::voxel<int16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<uint16_t>::staticID:
			Chunk::voxel<uint16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<int32_t>::staticID:
			Chunk::voxel<int32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<uint32_t>::staticID:
			Chunk::voxel<uint32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<float>::staticID:
			Chunk::voxel<float>( first, second, third, fourth ) = value;
			return true;
			break;
		case ValuePtr<double>::staticID:
			Chunk::voxel<double>( first, second, third, fourth ) = value;
			return true;
			break;
		default:
			return false;
		}
	}

	bool _setVoxel( const isis::util::ivector4 &coord, const float &value ) {
		return _setVoxel( coord[0], coord[1], coord[2], coord[3], value );
	}

	bool _convertToType( const unsigned short ID ) {
		return convertToType( ID );
	}

	bool _convertToType( const unsigned short ID, float scaling, size_t offset ) {
		return convertToType( ID, std::make_pair<util::Value<float>, util::Value<size_t> >( scaling, offset ) );
	}

private:
	PyObject *self;
};
}
}
}
#endif /* CHUNK_HPP_ */
