/*
 * common.hpp
 *
 *  Created on: Oct 20, 2010
 *      Author: tuerke
 */

#ifndef PYTHON_COMMON_HPP
#define PYTHON_COMMON_HPP

#include "CoreUtils/log.hpp"

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

} //namespace python

} //namespace isis
/** @} */
#endif /* PYTHON_COMMON_HPP_ */


