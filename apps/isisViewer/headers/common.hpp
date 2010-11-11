/*
 * common.hpp
 *
 *  Created on: Nov 2, 2010
 *      Author: tuerke
 */

#ifndef VIEWER_COMMON_HPP_
#define VIEWER_COMMON_HPP_

#include <vtkMatrix4x4.h>

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

template<typename T>
isis::util::vector4<T> mapCoordinates( const vtkMatrix4x4 *matrix, const isis::util::vector4<T> &coords, const isis::util::ivector4 &size)
{
	isis::util::fvector4 tmpRead = isis::util::fvector4(matrix->GetElement(0, 0), matrix->GetElement(0, 1),matrix->GetElement(0, 2), 0 );
	isis::util::fvector4 tmpPhase = isis::util::fvector4(matrix->GetElement(1, 0), matrix->GetElement(1, 1),matrix->GetElement(1, 2), 0 );
	isis::util::fvector4 tmpSlice = isis::util::fvector4(matrix->GetElement(2, 0), matrix->GetElement(2, 1),matrix->GetElement(2, 2), 0 );
	unsigned short biggestElemRead = getBiggestVecElem<T>(tmpRead);
	unsigned short biggestElemPhase = getBiggestVecElem<T>(tmpPhase);
	unsigned short biggestElemSlice = getBiggestVecElem<T>(tmpSlice);
	T x = biggestElemRead > 0 ? coords[biggestElemRead] : size[biggestElemRead] - coords[biggestElemRead];
	T y = biggestElemPhase > 0 ? coords[biggestElemPhase] : size[biggestElemPhase] - coords[biggestElemPhase];
	T z = biggestElemSlice > 0 ? coords[biggestElemSlice] : size[biggestElemSlice] - coords[biggestElemSlice];
	isis::util::vector4<T> retVec = isis::util::vector4<T>(x,y,z,0);
	return retVec;

}

}}

#endif /* VIEWER_COMMON_HPP_ */
