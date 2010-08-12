#include <boost/python.hpp>

#include "_application.hpp"

using namespace boost::python;
using namespace isis::python;

BOOST_PYTHON_MODULE( coreutils )
{

	//class Application
	class_<isis::util::Application, _Application>( "Application", init<const char *>() )
	//virtual void printHelp()const;
	.def( "printHelp", &isis::util::Application::printHelp )
	//static const std::string getCoreVersion( void );
	.def( "getCoreVersion", &isis::util::Application::getCoreVersion )
	.staticmethod( "getCoreVersion" )
	//virtual bool init( int argc, char **argv, bool exitOnError = true );
	.def( "init", &_Application::init )
	;
}
