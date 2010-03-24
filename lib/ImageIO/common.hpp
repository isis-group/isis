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
namespace isis{

namespace image_io{
typedef ImageIoLog Runtime;
typedef ImageIoDebug Debug;

template<typename HANDLE> void enable_log(LogLevel level){
	ENABLE_LOG(Runtime,HANDLE,level);
	ENABLE_LOG(Debug,HANDLE,level);
}
} //namespace image_io

} //namespace isis
/** @} */
#endif /* COMMON_HPP_ */

