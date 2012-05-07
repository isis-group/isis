/*
 * _writingvalueadapter.hpp
 *
 *  Created on: May, 07 2012
 *      Author: tuerke
 */

#ifndef _WRITINGVALUEADAPTER_HPP
#define _WRITINGVALUEADAPTER_HPP

#include "DataStorage/valuearray_base.hpp"
#include "common.hpp"
#include "../core/_convertToPython.hpp"
#include "../core/_convertFromPython.hpp"
#include "_types.hpp"
#include "CoreUtils/vector.hpp"

using namespace isis::data;

namespace isis
{
namespace python
{
namespace data
{

class _WritingValueAdapter : public isis::data::_internal::WritingValueAdapter, boost::python::wrapper<isis::data::_internal::WritingValueAdapter>
{
public:
	_WritingValueAdapter ( PyObject *p, const isis::data::_internal::WritingValueAdapter &base );

	api::object _get( ) const;
	void _set( const api::object & );

private:
	PyObject *self;

};
}}}
#endif