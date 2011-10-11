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
#include "_typeconvert.hpp"
#include <object.h>
using namespace isis::util;

namespace isis
{
namespace python
{


// helper class PropertyMap
class _PropertyMap : public PropertyMap, boost::python::wrapper< PropertyMap >
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


	void _setProperty( const std::string &key, PyObject *value ) {
		std::string typeName = value->ob_type->tp_name;
		if( std::find( _internal::knownTypes.begin(), _internal::knownTypes.end(), typeName ) == _internal::knownTypes.end() ) 
		{
			LOG( Runtime, error ) << "No conversion from type " << typeName << " available.";
		}
		
		util::istring ikey = key.c_str();
		//check for basic python types
		if( PyInt_Check( value ) ) {
			internSetProperty<int32_t>( ikey, value );
		} else if ( PyBool_Check( value ) ) {
			internSetProperty<bool>(ikey, value );
		} else if ( PyFloat_Check( value ) ) {
			internSetProperty<float>( ikey, value );
		} else if ( PyString_Check( value ) ) {
			internSetProperty<std::string>( ikey, value );
		} else if ( PyLong_Check( value ) ) {
			internSetProperty<int64_t>( ikey, value);
		}  else if ( typeName == std::string("ivector4") ) {
			internSetProperty<ivector4>(ikey, value);
		} else if ( typeName == std::string("fvector4") ){
			internSetProperty<fvector4>(ikey, value	);
		} else if ( typeName == std::string("dvector4") ) {
			internSetProperty<dvector4>(ikey, value);
		//we have a list....	
		} else if ( PyList_Check( value ) ) {
			if (!internSetPropertyList( ikey, value )) {
				LOG( Runtime, error ) << "Can not create a list of type " 
					<< PyList_GetItem(value, 0)->ob_type->tp_name << " !";
			}
		} else {
			LOG( Runtime, error ) << "Type " << typeName << " is not known.";
		}
	}
	
	
	boost::python::api::object _getProperty( const std::string &key ) {
		return _internal::typesMap.at( propertyValue( key.c_str() )->getTypeID() )->convert( *propertyValue( key.c_str() ) );
		
	}
	
	

private:
	PyObject *self;
	template<typename TYPE>
	void internSetProperty( const util::istring &key, PyObject *value ) {
		util::Value<TYPE> val( static_cast<TYPE>(boost::python::extract<TYPE>( value ) ) );
		this->setPropertyAs<TYPE>(  key , val );
	}
	
	bool internSetPropertyList( const util::istring &key, PyObject *value ) {
		//guess what the type of the list is. For that we have to go through the whole list
		std::list<std::string> types;
		for(Py_ssize_t i = 0; i < PyList_Size( value ); i++ ) {
			types.push_back(PyList_GetItem(value,i)->ob_type->tp_name);
		}
		// and now insert the elements as the "highest" type we found
		if(std::find(types.begin(), types.end(), std::string("str") ) != types.end() ) {
			setPropertyAs<slist>(key, extractFromList<std::string>(value));
			return true;
		} else if ( std::find(types.begin(), types.end(), std::string("float")) != types.end() ) {
			setPropertyAs<dlist>(key, extractFromList<double>(value));
			return true;
		} else if ( std::find(types.begin(), types.end(), std::string("int")) != types.end() ) {
			setPropertyAs<ilist>(key, extractFromList<int32_t>(value));
			return true;
		}
		return false;
	}

	template<typename TYPE>
	std::list<TYPE> extractFromList( PyObject *value ) {
		std::list<TYPE> retList;
		for(Py_ssize_t i = 0; i < PyList_Size(value); i++) {
			retList.push_back( static_cast<TYPE>( boost::python::extract<TYPE>( PyList_GetItem(value, i) ) ) );
		}
		return retList;
	}
	
	
	
	
};
}
}
#endif /* _PROPMAP_HPP_ */
