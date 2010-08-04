
#include "CoreUtils/property.hpp"

using namespace isis::util;

namespace isis { namespace python {

class _PropertyValue : public PropertyValue
{
public:
	_PropertyValue( PyObject *p ) : self( p ) {}
	_PropertyValue( PyObject *p, const PropertyValue &base ) : PropertyValue( base ), self( p ) {}

	void setNeeded( bool _needed ) {
		needed() = _needed;
	}
	void set( boost::python::str val ) {
		static_cast<PropertyValue &>( *this ) = std::string( boost::python::extract<const char * >( val ) ) ;
	}
	boost::python::str toString() {
		boost::python::str pyString( static_cast<PropertyValue &>( *this ).toString().c_str() );
		return pyString;
	}

private:
	PyObject *self;

};
}
}
