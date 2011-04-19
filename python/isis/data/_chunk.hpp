/*
 * _chunk.hpp
 *
 *  Created on: Oct 21, 2010
 *      Author: tuerke
 */

#ifndef CHUNK_HPP_
#define CHUNK_HPP_

#include "DataStorage/chunk.hpp"

namespace isis
{
namespace python
{
class _Chunk : public isis::data::Chunk, boost::python::wrapper<isis::data::Chunk>
{
public:
	//  _Chunk ( PyObject *p ) : self( p ) {}
	_Chunk ( PyObject *p, const isis::data::Chunk &base ) : isis::data::Chunk( base ), self( p ) {}

	float _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		switch( getTypeID() ) {
		case data::ValuePtr<int8_t>::staticID:
			return isis::data::Chunk::voxel<int8_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<uint8_t>::staticID:
			return isis::data::Chunk::voxel<uint8_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<int16_t>::staticID:
			return isis::data::Chunk::voxel<int16_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<uint16_t>::staticID:
			return isis::data::Chunk::voxel<uint16_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<int32_t>::staticID:
			return isis::data::Chunk::voxel<int32_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<uint32_t>::staticID:
			return isis::data::Chunk::voxel<uint32_t>( first, second, third, fourth );
			break;
		case data::ValuePtr<float>::staticID:
			return isis::data::Chunk::voxel<float>( first, second, third, fourth );
			break;
		case data::ValuePtr<double>::staticID:
			return isis::data::Chunk::voxel<double>( first, second, third, fourth );
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
		case data::ValuePtr<int8_t>::staticID:
			isis::data::Chunk::voxel<int8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<uint8_t>::staticID:
			isis::data::Chunk::voxel<uint8_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<int16_t>::staticID:
			isis::data::Chunk::voxel<int16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<uint16_t>::staticID:
			isis::data::Chunk::voxel<uint16_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<int32_t>::staticID:
			isis::data::Chunk::voxel<int32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<uint32_t>::staticID:
			isis::data::Chunk::voxel<uint32_t>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<float>::staticID:
			isis::data::Chunk::voxel<float>( first, second, third, fourth ) = value;
			return true;
			break;
		case data::ValuePtr<double>::staticID:
			isis::data::Chunk::voxel<double>( first, second, third, fourth ) = value;
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
#endif /* CHUNK_HPP_ */
