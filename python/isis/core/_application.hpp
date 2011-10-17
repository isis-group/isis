

#ifndef _APPLICATION_HPP_
#define _APPLICATION_HPP_

#include "CoreUtils/application.hpp"
#include "CoreUtils/progparameter.hpp"
#include "CoreUtils/types.hpp"

#include "_convertFromPython.hpp"
#include "_convertToPython.hpp"

#include <boost/python.hpp>

using namespace boost::python;

namespace isis
{
namespace python
{
namespace core
{
namespace Application {
	void _addParameter( isis::util::Application &base, const std::string &name, api::object value );
	
	api::object _getParameter( const isis::util::Application &base, const std::string &name );
	
	void _setDescription( isis::util::Application &base, const std::string &name, const std::string &desc );
	
	void _setHidden( isis::util::Application &base, const std::string &name, const bool &hidden );
	
	void _setNeeded( isis::util::Application &base, const std::string &name, const bool &needed );

	bool _init( isis::util::Application &base, int argc, list pyargv, bool exitOnError = true );
}

}
}
}
#endif
