
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

	virtual bool init( int argc, boost::python::list pyargv, bool exitOnError = true ) {
		char *argv[argc];
		size_t n = boost::python::len( pyargv );

		for( size_t i = 0; i < n; i++ ) {
			argv[i] = boost::python::extract<char *>( pyargv[i] );
		}

		return Application::init( argc, argv, exitOnError );
	}



private:
	PyObject *self;

};
}
}
