/*
 * _property.hpp
 *
 *  Created on: Oct 22, 2010
 *      Author: tuerke
 */

#ifndef _PROPERTY_HPP_
#define _PROPERTY_HPP_

using namespace isis::util;

namespace isis
{
namespace python
{


class _TypeValue : public PropertyValue, boost::python::wrapper< PropertyValue >
{

public:
	_TypeValue () {}
	_TypeValue ( PyObject *p ) : boost::python::wrapper< PropertyValue >(),  self( p ) {}
	_TypeValue ( PyObject *p, const PropertyValue &base ) : PropertyValue( base ),  boost::python::wrapper< PropertyValue >(), self( p ) {}

	std::string _toString( bool label ) { return this->toString( label ); }
private:
        PyObject *self;
	
};
}
}


#endif /* _PROPERTY_HPP_ */
