/*
 * datastorage.cpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#include <boost/python.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "common.hpp"
#include "_ioapplication.hpp"
#include "_image.hpp"
#include "std_item.hpp"


using namespace boost::python;
using namespace isis::python;

BOOST_PYTHON_MODULE( _data )
{

	//#######################################################################################
//	IOApplication
//#######################################################################################
	class_<isis::data::IOApplication, _IOApplication, bases<isis::util::Application> > ( "IOApplication", init<const char *, bool, bool>() )
		.def( "init", &_IOApplication::init )
		.def( "addParameter", &_IOApplication::_addParameter)
		.def( "setNeeded", &_IOApplication::_setNeeded)
		.def( "setHidden", &_IOApplication::_setHidden)
		.def( "printHelp", &isis::util::Application::printHelp )
		.def( "getCoreVersion", &isis::util::Application::getCoreVersion )
		.staticmethod( "getCoreVersion" )
		.def( "autoload", &isis::data::IOApplication::autoload )
		.def( "autowrite", &_IOApplication::_autowrite )
		.def( "images", &_IOApplication::_images)
	;

//#######################################################################################
//	Image
//#######################################################################################

	class_<isis::data::Image, _Image> ("Image", init<>() )
		.def( "checkMakeClean", &isis::data::Image::checkMakeClean)
		.def( "voxel",(float ( ::_Image::* )( const isis::util::ivector4& ) ) ( &_Image::_voxel), ( arg("coord") ))
		.def( "voxel",(float ( ::_Image::* )( const int&, const int&, const int&, const int& ) ) ( &_Image::_voxel), ( arg("first"),arg("second"), arg("third"), arg("fourth") ))
		.def( "setVoxel", (bool ( ::_Image::* )( const isis::util::ivector4&, const float& ) ) ( &_Image::_setVoxel), ( arg("coord"), arg("value") ))
		.def( "setVoxel", (bool ( ::_Image::* )( const int&, const int&, const int&, const int&, const float& ) ) ( &_Image::_setVoxel), ( arg("first"),arg("second"), arg("third"), arg("fourth"), arg("value") ))
		.def( "sizeToVector", &_Image::_sizeToVector)
			;

//#######################################################################################
//	ImageList
//#######################################################################################
	typedef std::list<isis::data::Image> IList;
	class_< IList > ("ImageList", init<IList>())
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
	class_<isis::data::Chunk> ("Chunk", no_init )
			;

}

