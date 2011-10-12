/*
 * _ioapplication.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef _IOAPPLICATION_HPP_
#define _IOAPPLICATION_HPP_

#include "DataStorage/io_application.hpp"
#include "core/_application.hpp"

namespace isis
{
namespace python
{

// helper class ioapplication
class _IOApplication : public data::IOApplication, boost::python::wrapper<data::IOApplication>
{

public:
	_IOApplication( PyObject *p, const char name[], const bool &input, const bool &output ) : data::IOApplication( name, input, output ),  boost::python::wrapper<data::IOApplication>(), self( p ) {}
	_IOApplication( PyObject *p, const data::IOApplication &base ) : data::IOApplication( base ),  boost::python::wrapper<data::IOApplication>(), self( p ) {}

	bool _autowrite( const std::list<isis::data::Image> &imgList, bool exitOnError ) {
		return isis::data::IOApplication::autowrite( imgList, exitOnError );
	}
	std::list<data::Image> _images() {
		return this->images;
	}

private:
	PyObject *self;

};
}
}
#endif /* _IOAPPLICATION_HPP_ */
