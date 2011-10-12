/*
 * _iofactory.hpp
 *
 *  Created on: Feb 2, 2011
 *      Author: tuerke
 */

#ifndef _IOFACTORY_HPP_
#define _IOFACTORY_HPP_

#include <DataStorage/io_factory.hpp>

using namespace isis::data;

namespace isis
{
namespace python
{
namespace data
{
// helper class iofactory
class _IOFactory
{
public:
	_IOFactory( PyObject *p ) : self( p ) {}
	_IOFactory() {}

	static bool _writeImage( const Image &img, const std::string &path, const std::string &suffix_override, const std::string &dialect ) {
		return IOFactory::write( img, path, suffix_override, dialect );
	}

	static bool _writeImageList( const std::list<Image>& images, const std::string &path, const std::string &suffix_override, const std::string &dialect ) {
		return IOFactory::write( images, path, suffix_override, dialect );
	}

	std::list<Image> _loadImageList ( const std::string &path, const std::string &suffix_override, const std::string &dialect ) {
		return IOFactory::load( path, suffix_override, dialect );
	}

	std::list<Chunk> _loadChunkList ( const std::string &path, const std::string &suffix_override, std::string &dialect ) {
		std::list<Chunk> tmpChunkList;
		IOFactory::load( tmpChunkList, path, suffix_override, dialect );
		return tmpChunkList;
	}

	std::list<Image> _chunkListToImageList( std::list<Chunk> &chunkList ) {
		return IOFactory::chunkListToImageList( chunkList );
	}

private:
	PyObject *self;

};


}
}
} //end namespace

#endif
