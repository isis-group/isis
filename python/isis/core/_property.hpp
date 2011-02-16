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
	_TypeValue ( PyObject *p ) : self( p ),  boost::python::wrapper< PropertyValue >() {}
	_TypeValue ( PyObject *p, const PropertyValue &base ) : PropertyValue( base ), self( p ),  boost::python::wrapper< PropertyValue >() {}

	std::string _toString( bool label ) { return this->toString( label ); }
private:
	PyObject *self;
};
}
}


#endif /* _PROPERTY_HPP_ */
