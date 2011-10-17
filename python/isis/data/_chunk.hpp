/*
 * _chunk.hpp
 *
 *  Created on: Oct 21, 2010
 *      Author: tuerke
 */

#ifndef CHUNK_HPP_
#define CHUNK_HPP_

#include "DataStorage/chunk.hpp"
#include <boost/python.hpp>
#include "../core/_convertToPython.hpp"
#include "common.hpp"

using namespace isis::data;

namespace isis
{
namespace python
{
namespace data
{

class _Chunk : public Chunk, boost::python::wrapper<Chunk>
{
public:
	//  _Chunk ( PyObject *p ) : self( p ) {}
	_Chunk ( PyObject *p, const Chunk &base ) : Chunk( base ), self( p ) {
		
	}

	boost::python::api::object _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		return isis::python::data::_internal::VoxelOp::getVoxelAsPyObject( *this, first, second, third, fourth );
	}

	boost::python::api::object _voxel( const isis::util::ivector4 &coord ) {
		return _voxel( coord[0], coord[1], coord[2], coord[3] );
	}

	bool _setVoxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const api::object &value ) {
		return isis::python::data::_internal::VoxelOp::setVoxelAsPyObject( *this, first, second, third, fourth, value );
	}
	

	bool _setVoxel( const isis::util::ivector4 &coord, const api::object &value ) {
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
