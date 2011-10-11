/*
 * _propmap.hpp
 *
 *  Created on: Oct 22, 2010
 *      Author: tuerke
 */

#ifndef _PROPMAP_HPP_
#define _PROPMAP_HPP_

#include "CoreUtils/propmap.hpp"
#include "boost/python.hpp"
#include "_convertFromPython.hpp"
#include "_convertToPython.hpp"
#include <object.h>

using namespace boost::python;

namespace isis
{
namespace python
{


// helper class PropertyMap
class _PropertyMap : public util::PropertyMap, boost::python::wrapper< util::PropertyMap >
{

public:
	_PropertyMap () : boost::python::wrapper< PropertyMap >() { _internal::createTypeMap(); }
	_PropertyMap ( PyObject *p ) : boost::python::wrapper< PropertyMap >(), self( p ) { _internal::createTypeMap(); }
	_PropertyMap ( PyObject *p, const PropertyMap &base ) : PropertyMap( base ), boost::python::wrapper< PropertyMap >(), self( p ) { _internal::createTypeMap(); }

	isis::util::PropertyMap _branch ( const util::istring &key ) {
		return this->branch( key );
		return this->branch( key );
	}

	isis::util::PropertyValue _propertyValue( const util::istring &key ) {
		return this->propertyValue( key );
	}


	void _setProperty( const std::string &key, api::object value ) {
		propertyValue(key.c_str()) = m_Converter.convert(value);		
	}
	
	
	api::object _getProperty( const std::string &key ) {
		return _internal::typesMap.at( propertyValue( key.c_str() )->getTypeID() )->convert( *propertyValue( key.c_str() ) );
		
	}
	
	

private:
	PyObject *self;
	_internal::ConvertFromPython m_Converter;
	
	
	

	
	
	
	
	
};
}
}
#endif /* _PROPMAP_HPP_ */
