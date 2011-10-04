/*
 * _iofactory.hpp
 *
 *  Created on: Feb 2, 2011
 *      Author: tuerke
 */

#ifndef _IOFACTORY_HPP_
#define _IOFACTORY_HPP_

#include <DataStorage/io_factory.hpp>

namespace isis
{
namespace python
{

// helper class iofactory
class _IOFactory
{
public:
	_IOFactory( PyObject *p ) : self(p) {}
	_IOFactory() {}

	static bool _writeImage( const data::Image &img, const std::string &path, const std::string &suffix_override, const std::string &dialect ) {
		return data::IOFactory::write( img, path, suffix_override, dialect );
	}

	static bool _writeImageList( const std::list<data::Image>& images, const std::string &path, const std::string &suffix_override, const std::string &dialect ) {
		return data::IOFactory::write( images, path, suffix_override, dialect );
	}

	std::list<data::Image> _loadImageList ( const std::string &path, const std::string &suffix_override, const std::string &dialect ) {
		return isis::data::IOFactory::load( path, suffix_override, dialect );
	}

	std::list<data::Chunk> _loadChunkList ( const std::string &path, const std::string &suffix_override, std::string &dialect ) {
		std::list<data::Chunk> tmpChunkList;
		data::IOFactory::load( tmpChunkList, path, suffix_override, dialect );
		return tmpChunkList;
	}

	std::list<data::Image> _chunkListToImageList( std::list<data::Chunk> &chunkList ) {
		return data::IOFactory::chunkListToImageList( chunkList );
	}

private:
	PyObject *self;

};



}
} //end namespace

#endif
