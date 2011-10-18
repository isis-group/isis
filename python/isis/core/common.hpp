/*
 * common.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef PYTHON_COMMON_HPP
#define PYTHON_COMMON_HPP

#include "CoreUtils/log.hpp"
#include "CoreUtils/common.hpp"
#include "CoreUtils/type.hpp"

/*! \addtogroup python
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis
{

struct PythonLog {static const char *name() {return "Python";}; enum {use = _ENABLE_LOG};};
struct PythonDebug {static const char *name() {return "PythonDebug";}; enum {use = _ENABLE_DEBUG};};

namespace python
{
typedef PythonLog Runtime;
typedef PythonDebug Debug;

template<typename HANDLE> void enable_log( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}
namespace core
{
namespace _internal
{
using namespace isis::util;
enum types { BOOL = Value<bool>::staticID,
			 INT8_T = Value<int_fast8_t>::staticID,
			 UINT8_T = Value<uint8_t>::staticID,
			 INT16_T = Value<int16_t>::staticID,
			 UINT16_T = Value<uint16_t>::staticID,
			 INT32_T = Value<int32_t>::staticID,
			 UINT32_T = Value<uint32_t>::staticID,
			 INT64_T = Value<int64_t>::staticID,
			 UINT64_T = Value<uint64_t>::staticID,
			 FLOAT = Value<float>::staticID,
			 DOUBLE = Value<double>::staticID,
			 FVECTOR4 = Value<fvector4>::staticID,
			 DVECTOR4 = Value<dvector4>::staticID,
			 IVECTOR4 = Value<ivector4>::staticID,
			 ILIST = Value<ilist>::staticID,
			 DLIST = Value<dlist>::staticID,
			 SLIST = Value<slist>::staticID,
			 STDSTRING = Value<std::string>::staticID,
			 SELECTION = Value<Selection>::staticID,
			 COMPLEX_FLOAT = Value< std::complex<float> >::staticID,
			 COMPLEX_DOUBLE = Value< std::complex<double> >::staticID,
			 BOOST_PTIME = Value<boost::posix_time::ptime>::staticID,
			 BOOST_DATE = Value<boost::gregorian::date>::staticID
		   };
} // namespace _internal

} // namespace core

} //namespace python

} //namespace isis
/** @} */
#endif /* PYTHON_COMMON_HPP_ */


