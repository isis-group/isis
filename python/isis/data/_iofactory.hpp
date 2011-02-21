/*
 * _iofactory.hpp
 *
 *  Created on: Feb 2, 2011
 *      Author: tuerke
 */

#ifndef _IOFACTORY_HPP_
#define _IOFACTORY_HPP_

#include <DataStorage/io_factory.hpp>

namespace isis
{
namespace python
{

// helper class iofactory
class _IOFactory
{
public:
	//_IOFactory( PyObject *p ) : self(p) {}
	_IOFactory() {}

	static std::list<data::Image> _load( const std::string path, const std::string suffix_override, const std::string dialect ) {
		std::list<isis::data::Image> tmpImageList;
		data::ImageList tmpList = data::IOFactory::load( path, suffix_override, dialect );
		BOOST_FOREACH( std::list<boost::shared_ptr<isis::data::Image> >::const_reference ref, tmpList ) {
			tmpImageList.push_back( *ref );
		}
		return tmpImageList;
	}
private:
	PyObject *self;

};



}
} //end namespace

#endif