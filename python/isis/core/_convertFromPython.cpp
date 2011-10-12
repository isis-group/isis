#include "_convertFromPython.hpp"

namespace isis {
namespace python {
namespace _internal {

ConvertFromPython::ConvertFromPython()
: m_KnownTypes( util::stringToList<std::string>( "bool list str float double int ivector4 dvector4 fvector4 selection" ))
{}

	
	
util::PropertyValue ConvertFromPython::convert( api::object value ) {
	std::string typeName = value.ptr()->ob_type->tp_name;
	// check if we know this type
	if( std::find( m_KnownTypes.begin(), m_KnownTypes.end(), typeName ) == m_KnownTypes.end() ) 
	{
		LOG( util::Runtime, error ) << "No conversion from type " << typeName << " available.";
		return util::PropertyValue();
	}
	//check for basic python types
	if( PyInt_Check( value.ptr() ) ) {
		return getValue<int32_t>( value );
	} else if ( PyBool_Check( value.ptr() ) ) {
		return  getValue<bool>( value );
	} else if ( PyFloat_Check( value.ptr() ) ) {
		return getValue<float>( value );
	} else if ( PyString_Check( value.ptr() ) ) {
		return getValue<std::string>( value );
	} else if ( PyLong_Check( value.ptr() ) ) {
		return getValue<int64_t>( value);
	}  else if ( typeName == std::string("ivector4") ) {
		return getValue<util::ivector4>( value);
	} else if ( typeName == std::string("fvector4") ){
		return getValue<util::fvector4>( value	);
	} else if ( typeName == std::string("dvector4") ) {
		return getValue<util::dvector4>( value );
	//we have a list....	
	} else if ( PyList_Check( value.ptr() ) ) {
		return getList( value );
		
	} else {
		LOG( util::Runtime, error ) << "Type " << typeName << " is not known.";
	}
	return util::PropertyValue();
}


util::PropertyValue ConvertFromPython::getList( api::object value ) {
	std::list<std::string> types;
	for(Py_ssize_t i = 0; i < PyList_Size( value.ptr() ); i++ ) {
		types.push_back(PyList_GetItem( value.ptr(), i )->ob_type->tp_name);
	}
	if(std::find(types.begin(), types.end(), std::string("str") ) != types.end() ) {
		return util::Value< util::slist > ( extractFromList<std::string>( value ) );
	} else if(std::find(types.begin(), types.end(), std::string("float") ) != types.end() ) {
		return util::Value< util::dlist > ( extractFromList<double>( value ) );
	} else if(std::find(types.begin(), types.end(), std::string("int") ) != types.end() ) {
		return util::Value< util::ilist > ( extractFromList<int32_t>( value ) );
	} else {
		LOG( util::Runtime, error ) << "The list contains elements of type that can not be converted!";
	}
	return util::PropertyValue();
}

	
}}} // end namespace
