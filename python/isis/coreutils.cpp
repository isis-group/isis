#include <boost/python.hpp>

#include "_application.hpp"
#include "_vector.hpp"
#include "_image.hpp"
#include "std_item.hpp"
#include "CoreUtils/selection.hpp"

using namespace boost::python;
using namespace isis::python;

BOOST_PYTHON_MODULE( core )
{
//#######################################################################################
//	Application
//#######################################################################################
	class_<isis::util::Application, _Application>( "Application", init<const char *>() )
		//virtual void printHelp()const;
		.def( "printHelp", &isis::util::Application::printHelp )
		//static const std::string getCoreVersion( void );
		.def( "getCoreVersion", &isis::util::Application::getCoreVersion )
		.staticmethod( "getCoreVersion" )
		//virtual bool init( int argc, char **argv, bool exitOnError = true );
		.def( "init", &_Application::init )
		.def( "addParameter", &_Application::addParameter)
		.def( "setNeeded", &_Application::setNeeded)
		.def( "setHidden", &_Application::setHidden)
	;

//#######################################################################################
//	IOApplication
//#######################################################################################
	class_<isis::data::IOApplication, _IOApplication, bases<isis::util::Application> > ( "IOApplication", init<const char *, bool, bool>() )
		.def( "init", &_IOApplication::init )
		.def( "addParameter", &_IOApplication::addParameter)
		.def( "setNeeded", &_IOApplication::setNeeded)
		.def( "setHidden", &_IOApplication::setHidden)
		.def( "printHelp", &isis::util::Application::printHelp )
		.def( "getCoreVersion", &isis::util::Application::getCoreVersion )
		.staticmethod( "getCoreVersion" )
		.def( "autoload", &isis::data::IOApplication::autoload )
		.def( "autowrite", &_IOApplication::autowrite )
		.def( "images", &_IOApplication::images)
	;

//#######################################################################################
//	Image
//#######################################################################################
	class_<isis::data::Image, _Image> ("Image", init<>() )
		.def( "checkMakeClean", &isis::data::Image::checkMakeClean)
		.def( "voxel", &_Image::voxel)
		.def( "setVoxel", &_Image::setVoxel)
			;

//#######################################################################################
//	ImageList
//#######################################################################################
	typedef std::list<isis::data::Image> IList;
	class_< IList > ("ImageList" )
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

//#######################################################################################
//	Selection
//#######################################################################################
	class_<isis::util::Selection>("Selection", init<const char*>())
		.def( init<>())
			;

//#######################################################################################
//	Vector4
//#######################################################################################
	class_<isis::util::fvector4, _Vector4<float> >("fvector4", init<float, float, float, float>())
		.def( init<>())
		.def("__setitem__", &_Vector4<float>::setItem)
		.def("__getitem__", &_Vector4<float>::getItem)
		;
	class_<isis::util::ivector4, _Vector4<int32_t> >("ivector4", init<int32_t, int32_t, int32_t, int32_t>())
		.def( init<>())
		.def("__setitem__", &_Vector4<int32_t>::setItem)
		.def("__getitem__", &_Vector4<int32_t>::getItem)
		;
	class_<isis::util::dvector4, _Vector4<double> >("dvector4", init<double, double, double, double>())
		.def("__setitem__", &_Vector4<double>::setItem)
		.def("__getitem__", &_Vector4<double>::getItem)
		.def( init<>())
		;

}
