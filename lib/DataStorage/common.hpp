//
// C++ Interface: common
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef DATA_COMMON_HPP
#define DATA_COMMON_HPP

namespace isis{ 

/*! \addtogroup data
 *  Additional documentation for group `mygrp'
 *  @{
 */

namespace data{
/// @cond _hidden
	struct DataLog{enum {use_rel = _ENABLE_DATA_LOG};};
	struct DataDebug{enum {use_rel = _ENABLE_DATA_DEBUG};};
/// @endcond
enum dimensions{timeDim=0,sliceDim,phaseDim,readDim};
}
/** @} */
}
#endif //DATA_COMMON_HPP