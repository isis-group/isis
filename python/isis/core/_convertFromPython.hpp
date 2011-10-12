#ifndef CONVERTFROMPYTHON_HPP
#define CONVERTFROMPYTHON_HPP

#include "CoreUtils/type.hpp"
#include "boost/python.hpp"
#include "CoreUtils/property.hpp"

using namespace boost::python;

namespace isis {
namespace python {
namespace _internal {

	
class ConvertFromPython {
	
public:
	
	ConvertFromPython();
	util::PropertyValue convert( api::object value ); 
	
	std::list<std::string> getKnownTypes() const { return m_KnownTypes; }
	
private:
	template<typename TYPE>
	util::Value<TYPE> getValue( api::object value ) {
		return util::Value<TYPE>( static_cast<TYPE>(boost::python::extract<TYPE>( value ) ) );
	}
	
	util::PropertyValue getList( api::object value );
	
	template<typename TYPE>
	std::list<TYPE> extractFromList( api::object value ) {
		std::list<TYPE> retList;
		for(Py_ssize_t i = 0; i < PyList_Size(value.ptr()); i++) {
			retList.push_back( convert( api::object( handle<>(borrowed( PyList_GetItem( value.ptr(), i )  ) ) ) )->as<TYPE>() );
		}
		return retList;
	}
	std::list<std::string> m_KnownTypes;
};


}}} // end namespace



#endif