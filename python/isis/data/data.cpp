/*
 * datastorage.cpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef DATA_HPP_
#define DATA_HPP_

#include <boost/python.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "common.hpp"
#include "_ioapplication.hpp"
#include "_image.hpp"
#include "_chunk.hpp"
#include "_iofactory.hpp"
#include "std_item.hpp"

using namespace boost::python;
using namespace isis::python;

BOOST_PYTHON_MODULE( _data )
{
	//#######################################################################################
	//  IOApplication
	//#######################################################################################
	class_<isis::data::IOApplication, _IOApplication, bases< _Application> > ( "IOApplication", init<const char *, bool, bool>() )
	.def( "init", &_IOApplication::init )
	.def( "addParameter", &_IOApplication::_addParameter )
	.def( "getParameterAsString", &_IOApplication::_getParameterAsString )
	.def( "setNeeded", &_IOApplication::_setNeeded )
	.def( "setHidden", &_IOApplication::_setHidden )
	.def( "autoload", &isis::data::IOApplication::autoload )
	.def( "autowrite", &_IOApplication::_autowrite )
	.def( "images", &_IOApplication::_images )
	.def( "setDescroption" , &_IOApplication::_setDescription );
	;
	//#######################################################################################
	//  Image
	//#######################################################################################
	class_<isis::data::Image, _Image, bases<isis::util::PropertyMap> >( "Image", init<>() )
	.def( init<isis::data::Image>() )
	.def( "checkMakeClean", &isis::data::Image::checkMakeClean )
	.def( "getVoxel", ( float ( ::_Image:: * )( const isis::util::ivector4 & ) ) ( &_Image::_voxel ), ( arg( "coord" ) ) )
	.def( "getVoxel", ( float ( ::_Image:: * )( const size_t &, const size_t &, const size_t &, const size_t & ) ) ( &_Image::_voxel ), ( arg( "first" ), arg( "second" ), arg( "third" ), arg( "fourth" ) ) )
	.def( "setVoxel", ( bool ( ::_Image:: * )( const isis::util::ivector4 &, const float & ) ) ( &_Image::_setVoxel ), ( arg( "coord" ), arg( "value" ) ) )
	.def( "setVoxel", ( bool ( ::_Image:: * )( const size_t &, const size_t &, const size_t &, const size_t &, const float & ) ) ( &_Image::_setVoxel ), ( arg( "first" ), arg( "second" ), arg( "third" ), arg( "fourth" ), arg( "value" ) ) )
	.def( "getSizeAsVector", &_Image::_getSizeAsVector )
	.def( "getChunkList", &_Image::_getChunksAsVector )
	.def( "getChunksAsList", &_Image::_getChunksAsVector )
	.def( "getMajorTypeID", &isis::data::Image::getMajorTypeID )
	.def( "getChunkAt", &isis::data::Image::getChunkAt )
	.def( "getChunk", ( isis::data::Chunk ( ::isis::data::Image:: * )( size_t, size_t, size_t, size_t, bool ) ) ( &isis::data::Image::getChunk ), ( arg( "first" ), arg( "second" ), arg( "third" ), arg( "fourth" ), arg( "copy_metadata" ) ) )
	.def( "getChunk", ( isis::data::Chunk ( ::_Image:: * )( const isis::util::ivector4 &, bool ) ) ( &_Image::_getChunk ), ( arg( "coord" ), arg( "copy_metadata" ) ) )
	.def( "getChunkAs", ( isis::data::Chunk ( ::_Image:: * )( const size_t &, const size_t &, const size_t &, const size_t &, const std::string & ) ) ( &_Image::_getChunk ), ( arg( "first" ), arg( "second" ), arg( "third" ), arg( "fourth" ), arg( "type" ) ) )
	.def( "getChunkAs", ( isis::data::Chunk ( ::_Image:: * )( const isis::util::ivector4 &, const std::string & ) ) ( &_Image::_getChunk ), ( arg( "coords" ), arg( "type" ) ) )
	.def( "insertChunk", &isis::data::Image::insertChunk )
	.def( "reIndex", &isis::data::Image::reIndex )
	.def( "isEmpty", &isis::data::Image::isEmpty )
	.def( "bytesPerVoxel", &isis::data::Image::getBytesPerVoxel )
	.def( "getMin", &_Image::_getMin )
	.def( "getMax", &_Image::_getMax )
	.def( "compare", &isis::data::Image::compare )
	.def( "transformCoords", &_Image::_transformCoords )
	.def( "getMainOrientation", &_Image::_getMainOrientation )
	.def( "convertToType", &isis::data::Image::convertToType )
	.def( "makeOfTypeName", &_Image::_makeOfTypeName )
	.def( "spliceDownTo", &_Image::_spliceDownTo )
	.def( "deepCopy", ( isis::data::Image ( ::_Image:: * )( void ) ) ( &_Image::_deepCopy ) )
	.def( "deepCopy", ( isis::data::Image ( ::_Image:: * )( std::string ) ) ( &_Image::_deepCopy ), ( arg( "type" ) ) )
	.def( "cheapCopy", ( isis::data::Image ( ::_Image:: * )( void ) ) ( &_Image::_cheapCopy ) )
	;
	//#######################################################################################
	//  ImageList
	//#######################################################################################
	typedef std::list<isis::data::Image> IList;
	class_< IList > ( "ImageList", init<IList>() )
	.def( init<>() )
	.def( "__len__", &IList::size )
	.def( "clear", &IList::clear )
	.def( "append", &std_list<IList>::add, with_custodian_and_ward<1, 2>() )
	.def( "__getitem__", &std_list<IList>::get, return_value_policy<copy_non_const_reference>() )
	.def( "__setitem__", &std_list<IList>::set, with_custodian_and_ward<1, 2>() )
	.def( "__delitem__", &std_list<IList>::del )
	;
	//#######################################################################################
	//  Chunk
	//#######################################################################################
	class_<isis::data::Chunk, _Chunk> ( "Chunk", init<_Chunk>() )
	.def( "getVoxel", ( float ( ::_Chunk:: * )( const isis::util::ivector4 & ) ) ( &_Chunk::_voxel ), ( arg( "coord" ) ) )
	.def( "getVoxel", ( float ( ::_Chunk:: * )( const size_t &, const size_t &, const size_t &, const size_t & ) ) ( &_Chunk::_voxel ), ( arg( "first" ), arg( "second" ), arg( "third" ), arg( "fourth" ) ) )
	.def( "setVoxel", ( bool ( ::_Chunk:: * )( const isis::util::ivector4 &, const float & ) ) ( &_Chunk::_setVoxel ), ( arg( "coord" ), arg( "value" ) ) )
	.def( "setVoxel", ( bool ( ::_Chunk:: * )( const size_t &, const size_t &, const size_t &, const size_t &, const float & ) ) ( &_Chunk::_setVoxel ), ( arg( "first" ), arg( "second" ), arg( "third" ), arg( "fourth" ), arg( "value" ) ) )
	.def( "useCount", &isis::data::Chunk::useCount )
	.def( "cloneToNew", &isis::data::Chunk::cloneToNew )
	.def( "convertToType", ( bool ( ::_Chunk:: * )( const unsigned short ) ) ( &_Chunk::_convertToType ), ( arg( "ID" ) ) )
	.def( "convertToType", ( bool ( ::_Chunk:: * )( const unsigned short, float, size_t ) ) ( &_Chunk::_convertToType ), ( arg( "ID" ), arg( "scaling" ), arg( "offset" ) ) )
	;
	//#######################################################################################
	//  ChunkList
	//#######################################################################################
	typedef std::list<isis::data::Chunk> CList;
	class_< CList > ( "ChunkList", init<CList>() )
	.def( init<>() )
	.def( "__len__", &CList::size )
	.def( "clear", &CList::clear )
	.def( "append", &std_list<CList>::add, with_custodian_and_ward<1, 2>() )
	.def( "__getitem__", &std_list<CList>::get, return_value_policy<copy_non_const_reference>() )
	.def( "__setitem__", &std_list<CList>::set, with_custodian_and_ward<1, 2>() )
	.def( "__delitem__", &std_list<CList>::del )
	;
	//#######################################################################################
	//  IOFactory
	//#######################################################################################
	class_<_IOFactory>( "IOFactory", no_init )
 	.def( "writeImage", &_IOFactory::_writeImage )
	.staticmethod( "writeImage" )
	.def( "write", &_IOFactory::_writeImage )
	.staticmethod( "write" )
	.def( "writeImageList", &_IOFactory::_writeImageList )
	.staticmethod( "writeImageList" )
	.def( "loadImageList", &_IOFactory::_loadImageList )
	.staticmethod( "loadImageList" )
	.def( "load", &_IOFactory::_loadImageList )
	.staticmethod( "load" )
	.def( "loadChunkList", &_IOFactory::_loadChunkList )
	.staticmethod( "loadChunkList" )
	.def( "chunkListToImageList", &_IOFactory::_chunkListToImageList )
	.staticmethod( "chunkListToImageList" )
	;

}
#endif
