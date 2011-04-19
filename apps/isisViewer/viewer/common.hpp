/*
 * common.hpp
 *
 *  Created on: Nov 2, 2010
 *      Author: tuerke
 */

#ifndef VIEWER_COMMON_HPP_
#define VIEWER_COMMON_HPP_

#include <vector>
#include <DataStorage/chunk.hpp>
#include <boost/shared_ptr.hpp>

namespace isis
{

struct ViewerLog {static const char *name() {return "Viewer";}; enum {use = _ENABLE_LOG};};
struct ViewerDebug {static const char *name() {return "ViewerDebug";}; enum {use = _ENABLE_DEBUG};};

namespace viewer
{

// just some helper typedefs which we will need regularly
	enum PlaneOrientation { axial, sagittal, coronal };

typedef ViewerLog Runtime;
typedef ViewerDebug Debug;
template<typename HANDLE> void enable_log( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}

template<typename T>
size_t getBiggestVecElem( const isis::util::vector4<T> &vec )
{
	size_t biggestVecElem = 0;
	float tmpValue = 0;

	for ( size_t vecElem = 0; vecElem < 4; vecElem++ ) {
		if ( fabs( vec[vecElem] ) > fabs( tmpValue ) ) {
			biggestVecElem = vecElem;
			tmpValue = vec[vecElem];
		}
	}

	return biggestVecElem;
}
}
}

#endif /* VIEWER_COMMON_HPP_ */
