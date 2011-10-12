/*
 * _propmap.hpp
 *
 *  Created on: Oct 22, 2010
 *      Author: tuerke
 */

#ifndef _PROPMAP_HPP_
#define _PROPMAP_HPP_

#include "CoreUtils/propmap.hpp"
#include "boost/python.hpp"
#include "_convertFromPython.hpp"
#include "_convertToPython.hpp"
#include "CoreUtils/singletons.hpp"

using namespace boost::python;

namespace isis
{
namespace python
{


// helper class PropertyMap
class _PropertyMap : public util::PropertyMap, boost::python::wrapper< util::PropertyMap >
{

public:
	_PropertyMap ();
	_PropertyMap ( PyObject *p );
	_PropertyMap ( PyObject *p, const PropertyMap &base );

	isis::util::PropertyMap _branch ( const util::istring &key );

	isis::util::PropertyValue _propertyValue( const util::istring &key );

	void _setProperty( const std::string &key, api::object value );

	api::object _getProperty( const std::string &key );


private:
	PyObject *self;
	_internal::ConvertFromPython m_ConverterFromPyton;


};
}
}
#endif /* _PROPMAP_HPP_ */
