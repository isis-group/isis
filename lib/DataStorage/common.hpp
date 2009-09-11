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
}
/** @} */
}
#endif //DATA_COMMON_HPP