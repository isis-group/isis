

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

// helper class application
class _Application : public util::Application, boost::python::wrapper<util::Application>
{
public:
	_Application( PyObject *p, const char name[] );
	_Application( PyObject *p, const util::Application &base );

	//wrapper function to convert a python list into a **char
	virtual bool init( int argc, list pyargv, bool exitOnError = true );

	void _addParameter( const std::string &name, api::object value );

	api::object _getParameter( const std::string &name ) const;

	void _setNeeded( const std::string &name, const bool needed ) {
		parameters[name].needed() = needed;
	}

	void _setHidden( const std::string &name, const bool hidden ) {
		parameters[name].hidden() = hidden;
	}

	void _setDescription( const std::string &name, const std::string &desc ) {
		parameters[name].setDescription( desc );
	}


private:
	PyObject *self;
	template<typename TYPE>
	void internAddParameter ( const std::string name, PyObject *value, std::string type ) {
		util::Value<TYPE> val( static_cast<TYPE>( boost::python::extract<TYPE>( value ) ) );
		val.copyByID( util::getTransposedTypeMap( true, true )[type] );
		parameters[name] = val;
		parameters[name].needed() = false;
	}

	_internal::ConvertFromPython m_ConverterFromPython;
};


}
}
#endif
