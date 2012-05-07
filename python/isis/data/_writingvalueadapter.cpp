/*
 * _writingvalueadapter.cpp
 *
 *  Created on: May, 07 2012
 *      Author: tuerke
 */

#include "_writingvalueadapter.hpp"

namespace isis
{
namespace python
{
namespace data
{

_WritingValueAdapter::_WritingValueAdapter ( PyObject *p, const isis::data::_internal::WritingValueAdapter &base )
	: isis::data::_internal::WritingValueAdapter ( base ), boost::python::wrapper< isis::data::_internal::WritingValueAdapter >(), self ( p )
{}

object _WritingValueAdapter::_get ( ) const
{
	return isis::util::Singletons::get<isis::python::core::_internal::TypesMap, 10>().at ( ( *this )->getTypeID() )->convert ( ( *this ).operator * () );
}

void _WritingValueAdapter::_set ( const object &val )
{
	WritingValueAdapter::operator=( isis::python::core::_internal::ConvertFromPython::convert( val ) );
}

}}}