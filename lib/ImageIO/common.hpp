/**  Description:
 *
 *
 *
 * Author: Lydia Hellrung <hellrung@cbs.mpg.de> (C)2009
 *
 * Copyright: See COPYING file that comes with this distribution
 */
/*
 * common.hpp
 *
 *  Created on: Nov 30, 2009
 *      Author: hellrung
 */

#ifndef IMAGEIO_COMMON_HPP
#define IMAGEIO_COMMON_HPP

#include "CoreUtils/log_modules.hpp"
#include "CoreUtils/log.hpp"

/*! \addtogroup image_io
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis
{

namespace image_io
{
typedef ImageIoLog Runtime;
typedef ImageIoDebug Debug;

/**
 * Set logging level for the namespace image_io.
 * This logging level will be used for every LOG(Debug,...) and LOG(Runtime,...) within the image_io namespace.
 * This is affected by by the _ENABLE_LOG and _ENABLE_DEBUG settings of the current compile and won't have an
 * effect on the Debug or Runtime logging if the corresponding define is set to "0".
 * So if you compile with "-D_ENABLE_DEBUG=0" against a library which (for example) was comiled with "-D_ENABLE_DEBUG=1",
 * you won't be able to change the logging level of the debug messages of these library.
 */
template<typename HANDLE> void enable_log( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}
} //namespace image_io

} //namespace isis
/** @} */
#endif /* COMMON_HPP_ */

