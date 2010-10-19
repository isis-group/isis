
#include "CoreUtils/application.hpp"
#include "DataStorage/io_application.hpp"
#include "CoreUtils/progparameter.hpp"
#include "CoreUtils/types.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace python
{

// helper class application
class _Application : public util::Application, boost::python::wrapper<util::Application>
{
public:
	_Application( PyObject *p, const char name[] ) : util::Application( name ), self( p ) {}
	_Application( PyObject *p, const util::Application &base ) : util::Application( base ), self( p ) {}

	//wrapper function to convert a python list into a **char
	virtual bool init( int argc, boost::python::list pyargv, bool exitOnError = true ) {
		char *argv[argc];
		size_t n = boost::python::len( pyargv );

		for( size_t i = 0; i < n; i++ ) {
			argv[i] = boost::python::extract<char *>( pyargv[i] );
		}

		return Application::init( argc, argv, exitOnError );
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

	const std::list<isis::data::Image> images( void ) {
		std::list<isis::data::Image> tmpImageList;
		BOOST_FOREACH(std::list<boost::shared_ptr<isis::data::Image> >::const_reference ref, isis::data::IOApplication::images)
		{
			tmpImageList.push_back(*ref);
		}
		return tmpImageList;
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

}
}
