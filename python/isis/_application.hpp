
#include "CoreUtils/application.hpp"
#include "CoreUtils/progparameter.hpp"
#include <boost/python.hpp>

using namespace isis::util;

namespace isis
{
namespace python
{

// helper class application
class _Application : public Application
{
public:
	_Application( PyObject *p, const char name[] ) : Application( name ), self( p ) {}
	_Application( PyObject *p, const Application &base ) : Application( base ), self( p ) {}

	//wrapper function to convert a python list into a **char
	virtual bool init( int argc, boost::python::list pyargv, bool exitOnError = true ) {
		char *argv[argc];
		size_t n = boost::python::len( pyargv );

		for( size_t i = 0; i < n; i++ ) {
			argv[i] = boost::python::extract<char *>( pyargv[i] );
		}

		return Application::init( argc, argv, exitOnError );
	}

	void addParameter( const std::string name, PyObject* value, std::string type )
	{
		if(PyFloat_Check( value )) {
			internAddParameter<float>( name, value, type);
		} else if(PyBool_Check( value )) {
			internAddParameter<bool>( name, value, type);
		} else if(PyInt_Check( value )) {
			internAddParameter<int64_t>( name, value, type);
		} else {
			internAddParameter<std::string>( name, value, type);
		}
	}
private:
	PyObject *self;
	template<typename TYPE>
	void internAddParameter ( const std::string name, PyObject* value, std::string type ) {

//		data::TypePtr<TYPE> val = static_cast<TYPE>( boost::python::extract<TYPE>( value ) );
//		val.copyToNewByID( getTransposedTypeMap(true, true)[type] );

//		parameters[name] = val;

//		std::cout << val << std::endl;
	}

};
}
}
