
#ifndef CORE_HPP_
#define CORE_HPP_

#include <boost/python.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include "common.hpp"
#include "_application.hpp"
#include "_convertToPython.hpp"
#include "_vector.hpp"
#include "_propmap.hpp"
#include "CoreUtils/selection.hpp"
#include "CoreUtils/singletons.hpp"

using namespace boost::python;
using namespace isis::python::core;

BOOST_PYTHON_MODULE( _core )
{
	isis::util::Singletons::get<isis::python::core::_internal::TypesMap, 10>().create();
	//#######################################################################################
	//  Application
	//#######################################################################################
	class_<isis::util::Application>( "Application", init<const char *>() )
	//virtual void printHelp()const;
	.def( "printHelp", &isis::util::Application::printHelp )
	//static const std::string getCoreVersion( void );
	.def( "getCoreVersion", &isis::util::Application::getCoreVersion )
	.staticmethod( "getCoreVersion" )
	//virtual bool init( int argc, char **argv, bool exitOnError = true );
	.def( "init", &isis::python::core::Application::_init )
	.def( "addParameter", &isis::python::core::Application::_addParameter )
	.def( "getParameter", &isis::python::core::Application::_getParameter )
	.def( "setNeeded", &isis::python::core::Application::_setNeeded )
	.def( "setHidden", &isis::python::core::Application::_setHidden )
	.def( "setDescription", &isis::python::core::Application::_setDescription )
	;
	//#######################################################################################
	//  PropertyMap
	//#######################################################################################
	class_<isis::util::PropertyMap>( "PropertyMap", init<>() )
	.def( "hasProperty", &isis::util::PropertyMap::hasProperty )
	.def( "hasBranch", &isis::util::PropertyMap::hasBranch )
	.def( "getBranch", &isis::python::core::PropertyMap::_branch )
	.def( "remove", ( bool ( ::isis::util::PropertyMap:: * )( const isis::util::istring & ) ) ( &isis::util::PropertyMap::remove ), ( arg( "key" ) ) )
	.def( "remove", ( bool ( ::isis::util::PropertyMap:: * )( const isis::util::PropertyMap &, bool ) ) ( &isis::util::PropertyMap::remove ), ( arg( "removeMap" ), arg( "keep_needed" ) ) )
	.def( "isValid", &isis::util::PropertyMap::isValid )
	.def( "isEmpty", &isis::util::PropertyMap::isEmpty )
	.def( "setProperty", &isis::python::core::PropertyMap::_setProperty )
	.def( "getProperty", &isis::python::core::PropertyMap::_getProperty )
	.def( "setPropertyAs", &isis::python::core::PropertyMap::_setPropertyAs )
	;
	//#######################################################################################
	//  Selection
	//#######################################################################################
	class_<isis::util::Selection>( "Selection", init<const char *>() )
	.def( init<>() )
	;
	//#######################################################################################
	//  Vector4
	//#######################################################################################
	class_<isis::util::fvector4, _Vector4<float> >( "fvector4", init<float, float, float, float>() )
	.def( init<>() )
	.def( init<isis::util::fvector4>() )
	.def( "__setitem__", &_Vector4<float>::setItem )
	.def( "__getitem__", &_Vector4<float>::getItem )
	.def( "__iter__", iterator<isis::util::fvector4>() )
	;
	class_<isis::util::ivector4, _Vector4<int32_t> >( "ivector4", init<int32_t, int32_t, int32_t, int32_t>() )
	.def( init< isis::util::ivector4 >() )
	.def( init<>() )
	.def( "__setitem__", &_Vector4<int32_t>::setItem )
	.def( "__getitem__", &_Vector4<int32_t>::getItem )
	.def( "__iter__", iterator<isis::util::ivector4>() )
	;
	class_<isis::util::dvector4, _Vector4<double> >( "dvector4", init<double, double, double, double>() )
	.def( init< isis::util::dvector4>() )
	.def( "__setitem__", &_Vector4<double>::setItem )
	.def( "__getitem__", &_Vector4<double>::getItem )
	.def( "__iter__", iterator<isis::util::dvector4>() )
	.def( init<>() )
	;
	//#######################################################################################
	//  enums for types
	//#######################################################################################
	using namespace isis::python::core::_internal;
	enum_<types>( "types" )
	.value( "INT8_T", INT8_T )
	.value( "UINT8_T", UINT8_T )
	.value( "INT16_T", INT16_T )
	.value( "UINT16_T", UINT16_T )
	.value( "INT32_T", INT32_T )
	.value( "UINT32_T", UINT32_T )
	.value( "INT64_T", INT64_T )
	.value( "UINT64_T", UINT64_T )
	.value( "FLOAT", FLOAT )
	.value( "DOUBLE", DOUBLE )
	.value( "FVECTOR4", FVECTOR4 )
	.value( "IVECTOR4", IVECTOR4 )
	.value( "DVECTOR4", DVECTOR4 )
	.value( "ILIST", ILIST )
	.value( "SLIST", SLIST )
	.value( "DLIST", DLIST )
	.value( "STD_STRING", STDSTRING )
	.value( "STRING", STDSTRING )
	.value( "SELECTION", SELECTION )
	.value( "COMPLEX_FLOAT", COMPLEX_FLOAT )
	.value( "COMPLEX_DOUBLE", COMPLEX_DOUBLE )
	.value( "BOOST_PTIME", BOOST_PTIME )
	.value( "BOOST_DATE", BOOST_DATE )
	;

}
#endif
