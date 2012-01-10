/*
 * _image.hpp
 *
 *  Created on: Oct 19, 2010
 *      Author: tuerke
 */

#ifndef _IMAGE_HPP_
#define _IMAGE_HPP_

#include "_types.hpp"
#include "DataStorage/image.hpp"
#include "CoreUtils/vector.hpp"
#include <vector>
#include <boost/algorithm/string.hpp>
#include "../core/common.hpp"
#include "common.hpp"
#include "../core/_convertToPython.hpp"

#include "core/_propmap.hpp"
#include <CL/cl_platform.h>
#include "DataStorage/chunk.hpp"

using namespace isis::data;

namespace isis
{
namespace python
{
namespace data
{

class _Image : public Image, public boost::python::wrapper<Image>
{

public:

	_Image ( PyObject *p );
	_Image ( PyObject *p, const Image &base );

	api::object _voxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		const Chunk &ch = getChunk( first, second, third, fourth, false );
		return isis::python::data::_internal::VoxelOp::getVoxelAsPyObject( ch, first, second, third, fourth );
	}
	api::object _voxel( const isis::util::ivector4 &coord ) {
		return _voxel( coord[0], coord[1], coord[2], coord[3] );
	}


	bool _setVoxel( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const api::object &value ) {
		Chunk ch = getChunk( first, second, third, fourth, false );
		return isis::python::data::_internal::VoxelOp::setVoxelAsPyObject( ch, first, second, third, fourth, value );


	}

	bool _setVoxel( const isis::util::ivector4 &coord, const api::object &value ) {
		return _setVoxel( coord[0], coord[1], coord[2], coord[3], value );
	}

	std::list<Chunk> _getChunksAsVector( void );

	const isis::util::ivector4 _getSizeAsVector( ) {
		return this->getSizeAsVector();
	}

	Chunk _getChunk( const isis::util::ivector4 &coord, bool copy_metadata ) {
		return getChunk( coord[0], coord[1], coord[2], coord[3], copy_metadata );
	}

	Chunk _getChunkAs( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth, const std::string &type );

	Chunk _getChunkAs( const isis::util::ivector4 &coord, const std::string &type ) {
		return _getChunkAs( coord[0], coord[1], coord[2], coord[3], type );
	}

	api::object _getMin( ) {
		return  util::Singletons::get<isis::python::core::_internal::TypesMap, 10>().at(
					getMinMax().first->getTypeID() )->convert( *getMinMax().first );
	}
	api::object _getMax( ) {
		return  util::Singletons::get<isis::python::core::_internal::TypesMap, 10>().at(
					getMinMax().second->getTypeID() )->convert( *getMinMax().second );
	}

	std::string _getMainOrientationAsString();

	void _transformCoords( boost::python::list matrix, const bool &center );

	bool _makeOfType( isis::python::data::image_types type  );

	size_t _spliceDownTo( const isis::data::dimensions dims );

	Image _deepCopy();

	Image _deepCopy( isis::python::data::image_types type );

	Image _cheapCopy( void ) {
		return *this;
	}

	static Image _createImage(  isis::python::data::image_types type, const size_t &first, const size_t &second, const size_t &third, const size_t &fourth );

private:
	PyObject *self;

	template<typename TYPE>
	static Image _internCreateImage( const size_t &first, const size_t &second, const size_t &third, const size_t &fourth ) {
		data::MemChunk<TYPE> chunk ( first, second, third, fourth );
		chunk.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
		chunk.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 0, 0, 0 ) );
		chunk.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0, 1, 0, 0 ) );
		chunk.setPropertyAs<util::fvector4>( "sliceVec", util::fvector4( 0, 0, 1, 0 ) );
		chunk.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1, 1 ) );
		chunk.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4() );
		return Image( chunk );
	}

};


class _ImageList : public std::list<Image>
{


};

}
}
}

#endif /* _IMAGE_HPP_ */
