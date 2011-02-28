/*
 * _ioapplication.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef _IOAPPLICATION_HPP_
#define _IOAPPLICATION_HPP_

#include "DataStorage/io_application.hpp"
#include "core/_application.hpp"

namespace isis
{
namespace python
{

// helper class ioapplication
class _IOApplication : public data::IOApplication, boost::python::wrapper<data::IOApplication>
{
public:
	_IOApplication( PyObject *p, const char name[], const bool &input, const bool &output ) : data::IOApplication( name, input, output ), self( p ),  boost::python::wrapper<data::IOApplication>() {}
	_IOApplication( PyObject *p, const data::IOApplication &base ) : data::IOApplication( "", true, true ), self( p ),  boost::python::wrapper<data::IOApplication>() {}

	virtual bool init( int argc, boost::python::list pyargv, bool exitOnError = true ) {
		char *argv[argc];
		size_t n = boost::python::len( pyargv );

		for( size_t i = 0; i < n; i++ ) {
			argv[i] = boost::python::extract<char *>( pyargv[i] );
		}

		return IOApplication::init( argc, argv, exitOnError );
	}


	void _addParameter( const std::string &name, PyObject *value, const std::string &type ) {
		if( PyFloat_Check( value ) ) {
			internAddParameter<float>( name, value, type );
		} else if( PyBool_Check( value ) ) {
			internAddParameter<bool>( name, value, type );
		} else if( PyInt_Check( value ) ) {
			internAddParameter<int64_t>( name, value, type );
		} else if( PyString_Check( value ) ) {
			internAddParameter<std::string>( name, value, type );
		} else if ( boost::iequals( type, "ivector4" ) ) {
			internAddParameter<isis::util::ivector4>( name, value, type );
		} else if ( boost::iequals( type, "dvector4" ) ) {
			internAddParameter<isis::util::dvector4>( name, value, type );
		} else if ( boost::iequals( type, "fvector4" ) ) {
			internAddParameter<isis::util::fvector4>( name, value, type );
		} else if ( boost::iequals( type, "selection" ) ) {
			internAddParameter<isis::util::Selection>( name, value, type );
		} else {
			LOG( Runtime, error ) << "Value " << type << " is not registered.";
		}
	}

	std::string _getParameterAsString( const std::string name ) {
		return parameters[name];
	}

	void _setNeeded( const std::string name, const bool needed ) {
		parameters[name].needed() = needed;
	}

	void _setHidden( const std::string name, const bool hidden ) {
		parameters[name].hidden() = hidden;
	}

	const std::list<isis::data::Image> _images( void ) {
		std::list<isis::data::Image> tmpImageList;
		BOOST_FOREACH( std::list<boost::shared_ptr<isis::data::Image> >::const_reference ref, this->images ) {
			tmpImageList.push_back( *ref );
		}
		return tmpImageList;
	}

	bool _autowrite( const std::list<isis::data::Image> &imgList, bool exitOnError ) {
		isis::data::ImageList listToWrite;
		BOOST_FOREACH( std::list<isis::data::Image>::const_reference ref, imgList ) {
			listToWrite.push_back( boost::shared_ptr<isis::data::Image> ( new isis::data::Image ( ref ) ) );
		}
		return isis::data::IOApplication::autowrite( listToWrite, exitOnError );
	}

private:
	PyObject *self;
	template<typename TYPE>
	void internAddParameter ( const std::string name, PyObject *value, std::string type ) {
		util::Value<TYPE> val( static_cast<TYPE>( boost::python::extract<TYPE>( value ) ) );
		//      if(!type.empty()) {
		//          val.copyToNewByID( util::getTransposedTypeMap(true, true)[type] );
		//      }
		parameters[name] = val;
		parameters[name].needed() = false;
	}
};
}
}
#endif /* _IOAPPLICATION_HPP_ */
