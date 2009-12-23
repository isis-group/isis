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

#ifndef COMMON_HPP_
#define COMMON_HPP_

/*! \addtogroup image_io
*  Additional documentation for group `mygrp'
*  @{
*/
namespace isis{

namespace image_io{
/// @cond _hidden
	struct ImageIoLog{static const char* name(){return "ImageIO";};enum {use = _ENABLE_IMAGEIO_LOG};};
	struct ImageIoDebug{static const char* name(){return "ImageIO";};enum {use= _ENABLE_IMAGEIO_DEBUG};};
/// @endcond

} //namespace image_io

} //namespace isis
/** @} */
#endif /* COMMON_HPP_ */

