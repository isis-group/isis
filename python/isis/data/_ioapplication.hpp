/*
 * _ioapplication.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef IOAPPLICATION_HPP_
#define IOAPPLICATION_HPP_

#include "DataStorage/io_application.hpp"

namespace isis
{
namespace python
{

// helper class ioapplication
class _IOApplication : public data::IOApplication, boost::python::wrapper<data::IOApplication>
{
public:
	_IOApplication( PyObject *p, const char name[], const bool &input, const bool &output) : data::IOApplication( name, input, output ), self( p ) {}
	_IOApplication( PyObject *p, const data::IOApplication &base ) : data::IOApplication( "", true, true), self( p ) {}

	virtual bool init( int argc, boost::python::list pyargv, bool exitOnError = true ) {
		char *argv[argc];
		size_t n = boost::python::len( pyargv );

		for( size_t i = 0; i < n; i++ ) {
			argv[i] = boost::python::extract<char *>( pyargv[i] );
		}
		return IOApplication::init( argc, argv, exitOnError );
	}

	void addParameter( const std::string name, PyObject* value, std::string type)
	{
		if(PyFloat_Check( value )) {
			internAddParameter<float>( name, value, type);
		} else if(PyBool_Check( value )) {
			internAddParameter<bool>( name, value, type);
		} else if(PyInt_Check( value )) {
			internAddParameter<int64_t>( name, value, type);
		} else if(PyString_Check( value )) {
			internAddParameter<std::string>( name, value, type);
		} else {
			internAddParameter<util::fvector4>( name, value, type);
		}
	}
	void setNeeded( const std::string name, const bool needed )
	{
		parameters[name].needed() = needed;
	}

	void setHidden( const std::string name, const bool hidden )
	{
		parameters[name].hidden() = hidden;
	}

	const std::list<isis::data::Image> _images( void ) {
		std::list<isis::data::Image> tmpImageList;
		BOOST_FOREACH(std::list<boost::shared_ptr<isis::data::Image> >::const_reference ref, isis::data::IOApplication::images)
		{
			tmpImageList.push_back(*ref);
		}
		return tmpImageList;
	}

	bool _autowrite( const std::list<isis::data::Image> &imgList, bool exitOnError ) {
		isis::data::ImageList listToWrite;
		BOOST_FOREACH(std::list<isis::data::Image>::const_reference ref, imgList )
		{
			listToWrite.push_back( boost::shared_ptr<isis::data::Image> (new isis::data::Image (ref)));
		}
		return isis::data::IOApplication::autowrite(listToWrite, exitOnError );
	}

private:
	PyObject *self;
	template<typename TYPE>
	void internAddParameter ( const std::string name, PyObject* value, std::string type ) {
		util::Type<TYPE> val(static_cast<TYPE>( boost::python::extract<TYPE>( value ) ));
		val.copyToNewById( util::getTransposedTypeMap(true, true)[type] );
		parameters[name] = val;
		parameters[name].needed() = false;
	}
};
}}
#endif /* IOAPPLICATION_HPP_ */
