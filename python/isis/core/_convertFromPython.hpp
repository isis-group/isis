#ifndef CONVERTFROMPYTHON_HPP
#define CONVERTFROMPYTHON_HPP

#include "CoreUtils/type.hpp"
#include <boost/python.hpp>
#include "CoreUtils/property.hpp"
#include <boost/date_time/gregorian/gregorian_io.hpp>
// #include <boost/date_time/posix_time/posix_time_io.hpp>
#include <datetime.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::python;

namespace isis
{
namespace python
{
namespace core
{
namespace _internal
{

template<typename TYPE>
void getValueFromPyObject( util::Value<TYPE> &ref, api::object value )
{
	ref = util::Value<TYPE>( static_cast<TYPE>( boost::python::extract<TYPE>( value ) ) );
}

template<>
void getValueFromPyObject<boost::gregorian::date> ( util::Value<boost::gregorian::date> &ref, api::object value );

template<>
void getValueFromPyObject<boost::posix_time::ptime> ( util::Value<boost::posix_time::ptime> &ref, api::object value );


class ConvertFromPython
{

public:
	static const std::list<std::string> knownTypes;
	static util::PropertyValue convert( api::object value );

private:


	static util::PropertyValue getList( api::object value );

	template<typename TYPE>
	static std::list<TYPE> extractFromList( api::object value ) {
		std::list<TYPE> retList;

		for( Py_ssize_t i = 0; i < PyList_Size( value.ptr() ); i++ ) {
			retList.push_back( convert( api::object( handle<>( borrowed( PyList_GetItem( value.ptr(), i )  ) ) ) ).as<TYPE>() );
		}

		return retList;
	}

};

}
}
}
} // end namespace



#endif