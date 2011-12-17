#include "_application.hpp"

namespace isis
{
namespace python
{
namespace core
{
namespace Application
{
void _addParameter( isis::util::Application &base, const std::string &name, boost::python::api::object value )
{
	static_cast<util::PropertyValue &>( base.parameters[name] ) = _internal::ConvertFromPython::convert( value );
}

boost::python::api::object _getParameter( const isis::util::Application &base, const std::string &name )
{
	return util::Singletons::get<_internal::TypesMap, 10>().at(
			   base.parameters.at( name.c_str() )->getTypeID() )->convert( *base.parameters.at( name.c_str() ) );
}


void _setDescription( util::Application &base, const std::string &name, const std::string &desc )
{
	base.parameters.at( name ).setDescription( desc );
}

void _setHidden( util::Application &base, const std::string &name, const bool &hidden )
{
	base.parameters.at( name ).hidden() = hidden;
}

void _setNeeded( util::Application &base, const std::string &name, const bool &needed )
{
	base.parameters.at( name ).needed() = needed;
}

bool _init( util::Application &base, boost::python::list pyargv, bool exitOnError )
{
	size_t n = boost::python::len( pyargv );
	char *argv[n];

	for( size_t i = 0; i < n; i++ ) {
		argv[i] = boost::python::extract<char *>( pyargv[i] );
	}

	return base.init( n, argv, exitOnError );

}



} // end namespace Application

}
}
} // end namespace