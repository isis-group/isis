/*
 * _ioapplication.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef _IOAPPLICATION_HPP_
#define _IOAPPLICATION_HPP_

#include "DataStorage/io_application.hpp"
#include "DataStorage/image.hpp"
#include <boost/python.hpp>
#include "common.hpp"

namespace isis
{
namespace python
{
namespace data
{

// helper class ioapplication
class _IOApplication : public isis::data::IOApplication, boost::python::wrapper<isis::data::IOApplication>
{

public:
	_IOApplication( PyObject *p, const char name[], const bool &input, const bool &output );
	_IOApplication( PyObject *p, const isis::data::IOApplication &base );

	std::list<isis::data::Image> _images() {
		return images;
	}
	
	isis::data::Image _fetchImageAs( isis::python::data::_internal::image_types type );
	
	bool _autowrite( std::list<isis::data::Image> imageList ) {
		return autowrite( imageList );
	}
	bool _autowrite( isis::data::Image image ) {
		return autowrite(image);
	}

private:
	PyObject *self;

};


}
}
}
#endif /* _IOAPPLICATION_HPP_ */
