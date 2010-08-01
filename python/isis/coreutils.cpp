#include <boost/python.hpp>

#include "_application.hpp"
#include "_propertyvalue.hpp"

using namespace boost::python;

BOOST_PYTHON_MODULE(coreutils)
{

	class_<isis::util::PropertyValue, _PropertyValue>("PropertyValue", init<>())
			.def("setNeeded", &_PropertyValue::setNeeded)
			.def("set", &_PropertyValue::set)
			.def("toString", &_PropertyValue::toString)
			;


	class_<isis::util::ProgParameter, bases< isis::util::PropertyValue > >("ProgParameter", init<isis::util::ProgParameter>())
			.def(init<>())
			.def("setDescription", &isis::util::ProgParameter::setDescription)
			;

	class_<isis::util::Application, _Application>("Application", init<const char*>())

			.def("parameters", &_Application::getParameters)

			//virtual void printHelp()const;
			.def("printHelp", &isis::util::Application::printHelp)

			//static const std::string getCoreVersion( void );
			.def("getCoreVersion", &isis::util::Application::getCoreVersion)
	        .staticmethod("getCoreVersion")

	        //virtual bool init( int argc, char **argv, bool exitOnError = true );
	        .def("init", &_Application::init)
			;



	class_<isis::util::ParameterMap>("ParameterMap", init<>())
				;





}
