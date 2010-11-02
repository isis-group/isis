/*
 * common.hpp
 *
 *  Created on: Nov 2, 2010
 *      Author: tuerke
 */

#ifndef VIEWER_COMMON_HPP_
#define VIEWER_COMMON_HPP_

namespace isis {

struct ViewerLog {static const char *name() {return "Viewer";}; enum {use = _ENABLE_LOG};};
struct ViewerDebug {static const char *name() {return "ViewerDebug";}; enum {use = _ENABLE_DEBUG};};

namespace viewer {

typedef ViewerLog Runtime;
typedef ViewerDebug Debug;
template<typename HANDLE> void enable_log( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}
}}

#endif /* VIEWER_COMMON_HPP_ */
