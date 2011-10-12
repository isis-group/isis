#include "_propmap.hpp"


namespace isis
{
namespace python
{


_PropertyMap::_PropertyMap()
	: boost::python::wrapper< PropertyMap >()
{
	util::Singletons::get<_internal::TypesMap, 10>().create();
}

_PropertyMap::_PropertyMap( PyObject *p )
	: boost::python::wrapper< PropertyMap >(),
	  self( p )
{
	util::Singletons::get<_internal::TypesMap, 10>().create();
}

_PropertyMap::_PropertyMap( PyObject *p, const isis::util::PropertyMap &base )
	: PropertyMap( base ),
	  boost::python::wrapper< PropertyMap >(),
	  self( p )
{
	util::Singletons::get<_internal::TypesMap, 10>().create();
}

boost::python::api::object _PropertyMap::_getProperty( const std::string &key )
{
	return util::Singletons::get<_internal::TypesMap, 10>().at(
			   propertyValue( key.c_str() )->getTypeID() )->convert( *propertyValue( key.c_str() ) );

}

void _PropertyMap::_setProperty( const std::string &key, boost::python::api::object value )
{
	propertyValue( key.c_str() ) = m_ConverterFromPyton.convert( value );
}


util::PropertyMap _PropertyMap::_branch( const isis::util::istring &key )
{
	return branch( key );
}

util::PropertyValue _PropertyMap::_propertyValue( const isis::util::istring &key )
{
	return propertyValue( key );
}



}
} // end namespace