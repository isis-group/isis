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
namespace core
{

namespace PropertyMap
{

void _setProperty( isis::util::PropertyMap &base, const std::string &key, api::object value );

api::object _getProperty( const isis::util::PropertyMap &base, const std::string &key );


isis::util::PropertyMap _branch( const isis::util::PropertyMap &base, const std::string &key );

} // end namespace PropertyMap
}
}
}
#endif /* _PROPMAP_HPP_ */
