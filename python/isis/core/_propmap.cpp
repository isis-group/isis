#include "_propmap.hpp"


namespace isis
{
namespace python
{
namespace core
{
namespace PropertyMap
{

void _setPropertyAs(util::PropertyMap& base, const std::string& key, boost::python::api::object value, isis::python::core::types type)
{
	static_cast<isis::util::_internal::ValueBase::Reference&>( base.propertyValue( key.c_str() ) ) = _internal::ConvertFromPython::convert( value )->copyByID( static_cast<unsigned short>(type));
}

	
void _setProperty( util::PropertyMap &base, const std::string &key, boost::python::api::object value )
{
	base.propertyValue( key.c_str() ) = _internal::ConvertFromPython::convert( value );
}

api::object _getProperty( const isis::util::PropertyMap &base, const std::string &key )
{
	return util::Singletons::get<_internal::TypesMap, 10>().at(
			   base.propertyValue( key.c_str() )->getTypeID() )->convert( *base.propertyValue( key.c_str() ) );

}

util::PropertyMap _branch( const isis::util::PropertyMap &base, const std::string &key )
{
	return base.branch( key.c_str() );
}


} // end namespace PropertyMap

}
}
} // end namespace