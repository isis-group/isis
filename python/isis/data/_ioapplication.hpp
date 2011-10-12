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
namespace data {
	
// helper class ioapplication
class _IOApplication : public isis::data::IOApplication, boost::python::wrapper<isis::data::IOApplication>
{

public:
	_IOApplication( PyObject *p, const char name[], const bool &input, const bool &output );
	_IOApplication( PyObject *p, const isis::data::IOApplication &base );

	bool _autowrite( const std::list<isis::data::Image> &imgList, bool exitOnError ) {
		return isis::data::IOApplication::autowrite( imgList, exitOnError );
	}
	std::list<isis::data::Image> _images() {
		return images;
	}

private:
	PyObject *self;

};
}
}
}
#endif /* _IOAPPLICATION_HPP_ */
