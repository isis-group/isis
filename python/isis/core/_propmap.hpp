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


// helper class PropMap
class _PropMap : public PropMap, boost::python::wrapper< PropMap >
{
public:
	_PropMap () : boost::python::wrapper< PropMap >() {}
	_PropMap ( PyObject* p ) : self( p ), boost::python::wrapper< PropMap >() {}
	_PropMap ( PyObject* p, const PropMap& base ) : PropMap( base ), self( p ), boost::python::wrapper< PropMap >() {}

	isis::util::PropMap _branch ( const std::string& key ) {
		return this->branch( key );
	}

	isis::util::PropertyValue _propertyValue( const std::string& key )
	{
		return this->propertyValue( key );
	}

	void _setProperty( std::string key, PyObject* value, std::string type) {
		if(PyFloat_Check( value )) {
			internSetProperty<float>( key, value, type);
		} else if(PyBool_Check( value )) {
			internSetProperty<bool>( key, value, type);
		} else if(PyInt_Check( value )) {
			internSetProperty<int64_t>( key, value, type);
		} else if(PyString_Check( value )) {
			internSetProperty<std::string>( key, value, type);
		} else if (boost::iequals(type, "ivector4" ) ) {
			internSetProperty<isis::util::ivector4>( key, value, type);
		} else if (boost::iequals(type, "dvector4" ) ) {
			internSetProperty<isis::util::dvector4>( key, value, type);
		} else if (boost::iequals(type, "fvector4" ) ) {
			internSetProperty<isis::util::fvector4>( key, value, type);
		} else if (boost::iequals(type, "selection" )) {
			internSetProperty<isis::util::Selection>( key, value, type);
		} else {
			LOG(Runtime, error ) << "Type " << type << " is not registered.";
		}
	}

private:
	PyObject *self;
	template<typename TYPE>
	void internSetProperty ( const std::string key, PyObject* value, std::string type ) {
		util::Type<TYPE> val(static_cast<TYPE>( boost::python::extract<TYPE>( value ) ));
		val.copyToNewById( util::getTransposedTypeMap(true, true)[type] );
		this->setProperty<TYPE>(key, val);

	}
};
}}
#endif /* _PROPMAP_HPP_ */
