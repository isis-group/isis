
#include "CoreUtils/application.hpp"

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

	void addParameter( std::string name, std::string value ) {
		this->parameters[name] = value;
	}
	std::string getParameterValue( std::string name ) {
		return this->parameters.operator [](name);
	}



private:
	PyObject *self;

};
}
}
