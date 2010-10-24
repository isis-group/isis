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
#include "std_item.hpp"

using namespace boost::python;
using namespace isis::python;

BOOST_PYTHON_MODULE( _data )
{


//#######################################################################################
//	IOApplication
//#######################################################################################
	class_<isis::data::IOApplication, _IOApplication, bases< _Application> > ( "IOApplication", init<const char *, bool, bool>() )
		.def( "init", &_IOApplication::init )
		.def( "addParameter", &_IOApplication::_addParameter)
		.def( "setNeeded", &_IOApplication::_setNeeded)
		.def( "setHidden", &_IOApplication::_setHidden)
		.def( "autoload", &isis::data::IOApplication::autoload )
		.def( "autowrite", &_IOApplication::_autowrite )
		.def( "images", &_IOApplication::_images)
	;

//#######################################################################################
//	Image
//#######################################################################################

	class_<isis::data::Image, _Image, bases<isis::util::PropMap> >("Image", init<>() )
		.def( init<isis::data::Image>() )
		.def( "checkMakeClean", &isis::data::Image::checkMakeClean)
		.def( "getVoxel",(float ( ::_Image::* )( const isis::util::ivector4& ) ) ( &_Image::_voxel), ( arg("coord") ))
		.def( "getVoxel",(float ( ::_Image::* )( const size_t&, const size_t&, const size_t&, const size_t& ) ) ( &_Image::_voxel), ( arg("first"),arg("second"), arg("third"), arg("fourth") ))
		.def( "setVoxel", (bool ( ::_Image::* )( const isis::util::ivector4&, const float& ) ) ( &_Image::_setVoxel), ( arg("coord"), arg("value") ))
		.def( "setVoxel", (bool ( ::_Image::* )( const size_t&, const size_t&, const size_t&, const size_t&, const float& ) ) ( &_Image::_setVoxel), ( arg("first"),arg("second"), arg("third"), arg("fourth"), arg("value") ))
		.def( "sizeToVector", &_Image::_sizeToVector)
		.def( "getChunkList", &_Image::_getChunkList)
		.def( "typeID", &isis::data::Image::typeID )
		.def( "getChunkAt", &isis::data::Image::getChunkAt )
		.def( "getChunk", ( isis::data::Chunk ( ::isis::data::Image::* )( size_t, size_t, size_t, size_t, bool ) ) ( &isis::data::Image::getChunk), (arg("first"), arg("second"), arg("third"), arg("fourth"), arg("copy_metadata") ))
		.def( "getChunk", ( isis::data::Chunk ( ::_Image::* )( const isis::util::ivector4&, bool ) ) ( &_Image::_getChunk), (arg("coord"), arg("copy_metadata") ))
		.def( "getChunkAs", ( isis::data::Chunk ( ::_Image::* )( const size_t&, const size_t&, const size_t&, const size_t&, const std::string&) ) ( &_Image::_getChunk), (arg("first"), arg("second"), arg("third"), arg("fourth"), arg("type") ))
		.def( "getChunkAs", ( isis::data::Chunk ( ::_Image::* )( const isis::util::ivector4&, const std::string&) ) ( &_Image::_getChunk), ( arg("coords"), arg("type") ))
		.def( "insertChunk", &isis::data::Image::insertChunk)
		.def( "reIndex", &isis::data::Image::reIndex)
		.def( "empty", &isis::data::Image::empty)
		.def( "bytesPerVoxel", &isis::data::Image::bytes_per_voxel )
		.def( "getMin", &_Image::_getMin )
		.def( "getMax", &_Image::_getMax )
		.def( "cmp", &isis::data::Image::cmp)
		.def( "transformCoords", &_Image::_transformCoords )
		.def( "getMainOrientation", &_Image::_getMainOrientation )
		.def( "makeOfTypeId", &isis::data::Image::makeOfTypeId)
		.def( "makeOfTypeName", &_Image::_makeOfTypeName)
		.def( "spliceDownTo", &_Image::_spliceDownTo)
		.def( "deepCopy", ( isis::data::Image ( ::_Image::* )( void ) ) ( &_Image::_deepCopy ))
		.def( "deepCopy", ( isis::data::Image ( ::_Image::* )( std::string ) ) ( &_Image::_deepCopy ), ( arg("type")))
		.def( "cheapCopy", ( isis::data::Image ( ::_Image::* )( void ) ) ( &_Image::_cheapCopy ))
			;

//#######################################################################################
//	ImageList
//#######################################################################################
	typedef std::list<isis::data::Image> IList;
	class_< IList > ("ImageList", init<IList>())
		.def( init<>())
		.def("__len__", &IList::size )
		.def("clear", &IList::clear )
		.def("append", &std_list<IList>::add, with_custodian_and_ward<1,2>() )
		.def("__getitem__", &std_list<IList>::get, return_value_policy<copy_non_const_reference>() )
		.def("__setitem__", &std_list<IList>::set, with_custodian_and_ward<1,2>() )
		.def("__delitem__", &std_list<IList>::del )
			;


//#######################################################################################
//	Chunk
//#######################################################################################
	class_<isis::data::Chunk, _Chunk> ("Chunk", init<_Chunk>() )
			;

//#######################################################################################
//	ChunkList
//#######################################################################################
	typedef std::list<isis::data::Chunk> CList;
	class_< CList > ("ChunkList", init<CList>())
		.def( init<>())
		.def("__len__", &CList::size )
		.def("clear", &CList::clear )
		.def("append", &std_list<CList>::add, with_custodian_and_ward<1,2>() )
		.def("__getitem__", &std_list<CList>::get, return_value_policy<copy_non_const_reference>() )
		.def("__setitem__", &std_list<CList>::set, with_custodian_and_ward<1,2>() )
		.def("__delitem__", &std_list<CList>::del )
		;
}
#endif
