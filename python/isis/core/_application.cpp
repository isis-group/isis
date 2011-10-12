#include "_application.hpp"

namespace isis
{
namespace python
{
namespace core 
{

_Application::_Application( PyObject *p, const char name[] )
	: util::Application( name ),
	  boost::python::wrapper<util::Application>(),
	  self( p )
{
	util::Singletons::get<_internal::TypesMap, 10>().create();
}

_Application::_Application( PyObject *p, const isis::util::Application &base )
	: util::Application( base ),
	  boost::python::wrapper<util::Application>(),
	  self( p )
{
	util::Singletons::get<_internal::TypesMap, 10>().create();
}



bool _Application::init( int argc, list pyargv, bool exitOnError )
{
	char *argv[argc];
	size_t n = boost::python::len( pyargv );

	for( size_t i = 0; i < n; i++ ) {
		argv[i] = boost::python::extract<char *>( pyargv[i] );
	}

	return util::Application::init( argc, argv, exitOnError );
}

void _Application::_addParameter( const std::string &name, boost::python::api::object value )
{
	static_cast<util::PropertyValue &>( parameters[name] ) = m_ConverterFromPython.convert( value );
}

api::object _Application::_getParameter( const std::string &name ) const
{
	return util::Singletons::get<_internal::TypesMap, 10>().at(
			   parameters.at( name.c_str() )->getTypeID() )->convert( *parameters.at( name.c_str() ) );
}


}
}
} // end namespace