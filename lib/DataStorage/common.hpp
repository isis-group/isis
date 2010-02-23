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
	struct DataLog{static const char* name(){return "Data";};enum {use = _ENABLE_DATA_LOG};};
	struct DataDebug{static const char* name(){return "Data";};enum {use= _ENABLE_DATA_DEBUG};};
/// @endcond
	enum dimensions{readDim=0,phaseDim,sliceDim,timeDim};
}
/** @} */
}
#endif //DATA_COMMON_HPP
