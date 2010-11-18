

#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_

#include "CoreUtils/application.hpp"
#include "CoreUtils/progparameter.hpp"
#include "CoreUtils/types.hpp"


namespace isis
{
namespace python
{

// helper class application
class _Application : public util::Application, boost::python::wrapper<util::Application>
{
public:
	_Application( PyObject *p, const char name[] ) : util::Application( name ), self( p ), boost::python::wrapper<util::Application>() {}
	_Application( PyObject *p, const util::Application &base ) : util::Application( base ), self( p ), boost::python::wrapper<util::Application>() {}

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
		} else if (boost::iequals(type, "ivector4" ) ) {
			internAddParameter<isis::util::ivector4>( name, value, type);
		} else if (boost::iequals(type, "dvector4" ) ) {
			internAddParameter<isis::util::dvector4>( name, value, type);
		} else if (boost::iequals(type, "fvector4" ) ) {
			internAddParameter<isis::util::fvector4>( name, value, type);
		} else if (boost::iequals(type, "selection" )) {
			internAddParameter<isis::util::Selection>( name, value, type);
		} else {
			LOG(Runtime, error ) << "Type " << type << " is not registered.";
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
		val.copyToNewByID( util::getTransposedTypeMap(true, true)[type] );
		parameters[name] = val;
		parameters[name].needed() = false;
	}

};


}
}
#endif
