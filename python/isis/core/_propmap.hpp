/*
 * _propmap.hpp
 *
 *  Created on: Oct 22, 2010
 *      Author: tuerke
 */

#ifndef _PROPMAP_HPP_
#define _PROPMAP_HPP_

#include "CoreUtils/propmap.hpp"
using namespace isis::util;

namespace isis
{
namespace python
{


// helper class PropertyMap
class _PropertyMap : public PropertyMap, boost::python::wrapper< PropertyMap >
{
public:
	_PropertyMap () : boost::python::wrapper< PropertyMap >() {}
	_PropertyMap ( PyObject *p ) : self( p ), boost::python::wrapper< PropertyMap >() {}
	_PropertyMap ( PyObject *p, const PropertyMap &base ) : PropertyMap( base ), self( p ), boost::python::wrapper< PropertyMap >() {}

	isis::util::PropertyMap _branch ( const util::istring &key ) {
		return this->branch( key );
		return this->branch( key );
	}

	isis::util::PropertyValue _propertyValue( const util::istring &key ) {
		return this->propertyValue( key );
	}


	void _setPropertyAs( istring key, PyObject *value, std::string type ) {
		if( PyFloat_Check( value ) ) {
			internSetProperty<float>( key, value, type );
		} else if( PyBool_Check( value ) ) {
			internSetProperty<bool>( key, value, type );
		} else if( PyInt_Check( value ) ) {
			internSetProperty<int64_t>( key, value, type );
		} else if( PyString_Check( value ) ) {
			internSetProperty<std::string>( key, value, type );
		} else if ( boost::iequals( type, "ivector4" ) ) {
			internSetProperty<isis::util::ivector4>( key, value, type );
		} else if ( boost::iequals( type, "dvector4" ) ) {
			internSetProperty<isis::util::dvector4>( key, value, type );
		} else if ( boost::iequals( type, "fvector4" ) ) {
			internSetProperty<isis::util::fvector4>( key, value, type );
		} else if ( boost::iequals( type, "selection" ) ) {
			internSetProperty<isis::util::Selection>( key, value, type );
		} else {
			LOG( Runtime, error ) << "Value " << type << " is not registered.";
		}
	}

private:
	PyObject *self;
	template<typename TYPE>
	void internSetProperty ( const util::istring key, PyObject *value, std::string type ) {
		util::Value<TYPE> val( static_cast<TYPE>( boost::python::extract<TYPE>( value ) ) );
		val.copyByID( util::getTransposedTypeMap( true, true )[type] );
		this->setPropertyAs<TYPE>( key, val );
	}
};
}
}
#endif /* _PROPMAP_HPP_ */
