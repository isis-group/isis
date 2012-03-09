/*
 * _iofactory.hpp
 *
 *  Created on: Feb 2, 2011
 *      Author: tuerke
 */

#ifndef _IOFACTORY_HPP_
#define _IOFACTORY_HPP_

#include <DataStorage/io_factory.hpp>
#include <DataStorage/io_interface.h>
#include <boost/python.hpp>

struct stat;
using namespace isis::data;

namespace isis
{
namespace python
{
namespace data
{
// helper class iofactory
class _IOFactory
{
public:
	_IOFactory( PyObject *p ) : self( p ) {}
	_IOFactory() {}

	static std::list<Image> _load ( const std::string &path, const std::string &suffix_override, const util::istring &dialect ) {
		return IOFactory::load( path, suffix_override, dialect );
	}
	static std::list<Image> _load ( const std::string &path, const std::string &suffix_override ) {
		return IOFactory::load( path, suffix_override );
	}

	static std::list<Image> _load ( const std::string &path ) {
		return IOFactory::load( path );
	}

	static bool _write( const isis::data::Image &image, const std::string &path, const std::string &suffix_override, const util::istring &dialect ) {
		return IOFactory::write( image, path, suffix_override, dialect );
	}

	static bool _write( const isis::data::Image &image, const std::string &path, const std::string &suffix_override ) {
		return IOFactory::write( image, path, suffix_override, "" );
	}
	static bool _write( const isis::data::Image &image, const std::string &path ) {
		return IOFactory::write( image, path, "", "" );
	}
	static bool _write( std::list<isis::data::Image> &images, const std::string &path, const std::string &suffix_override, const util::istring &dialect ) {
		return IOFactory::write( images, path, suffix_override, dialect );
	}

	static bool _write( std::list<isis::data::Image> &images, const std::string &path, const std::string &suffix_override ) {
		return IOFactory::write( images, path, suffix_override, "" );
	}
	static bool _write( std::list<isis::data::Image> &images, const std::string &path ) {
		return IOFactory::write( images, path, "", "" );
	}

	static boost::python::list _getFormats() {
		isis::data::IOFactory::FileFormatList formatList = IOFactory::getFormats();
		boost::python::list retList;
		BOOST_FOREACH( isis::data::IOFactory::FileFormatList::const_reference formatRef, formatList ) {
			boost::python::list attrList;
			attrList.append<std::string>( formatRef->getName() );
			std::stringstream suffixList;
			BOOST_FOREACH( std::list< isis::util::istring>::const_reference suffix, formatRef->getSuffixes() ) {

				suffixList << suffix.c_str() << " ";
			}
			attrList.append< std::string >( suffixList.str() );
			retList.append<boost::python::list>( attrList );
		}
		return retList;

	}
private:
	PyObject *self;

};


}
}
} //end namespace

#endif
