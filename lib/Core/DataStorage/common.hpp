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

#include "../CoreUtils/log_modules.hpp"
#include "../CoreUtils/log.hpp"
#include "../CoreUtils/propmap.hpp"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>

namespace isis
{

/*! \addtogroup data
 *  Additional documentation for group `mygrp'
 *  @{
 */

namespace image_io
{
typedef ImageIoLog Runtime;
typedef ImageIoDebug Debug;

template<typename HANDLE> void enableLog( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}
} //namespace image_io

namespace data
{

namespace _internal
{
using namespace boost::numeric::ublas;


template <typename TYPE>
bool inverseMatrix( const matrix<TYPE> &inMatrix, matrix<TYPE> &inverse )
{
	matrix<TYPE> A( inMatrix );
	permutation_matrix<TYPE> pm( A.size1() );
	int res = lu_factorize( A, pm );

	if( res != 0 ) {
		return false;
	}

	inverse.assign( identity_matrix<TYPE>( inMatrix.size1() ) );
	lu_substitute( A, pm, inverse );
	return true;
}

bool transformCoords( isis::util::PropertyMap &, const isis::util::FixedVector<size_t, 4> size, boost::numeric::ublas::matrix<float>, bool transformCenterIsImageCenter = false );

}

typedef DataLog Runtime;
typedef DataDebug Debug;
enum dimensions {rowDim = 0, columnDim, sliceDim, timeDim};
enum scannerAxis { x = 0, y, z };

/**
 * Set logging level for the namespace data.
 * This logging level will be used for every LOG(Debug,...) and LOG(Runtime,...) within the data namespace.
 * This is affected by by the _ENABLE_LOG and _ENABLE_DEBUG settings of the current compile and won't have an
 * effect on the Debug or Runtime logging if the corresponding define is set to "0".
 * So if you compile with "-D_ENABLE_DEBUG=0" against a library which (for example) was comiled with "-D_ENABLE_DEBUG=1",
 * you won't be able to change the logging level of the debug messages of these library.
 */
template<typename HANDLE> void enableLog( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}
}
/** @} */
}
#endif //DATA_COMMON_HPP
