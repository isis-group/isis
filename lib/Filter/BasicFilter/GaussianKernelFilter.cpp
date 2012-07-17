#include "../filter.hpp"
#include "GaussianKernelFilter.hpp"
#include <cmath>

namespace isis
{
namespace filter
{

bool GaussianKernelFilter::process()
{
	const ValueType sigma = parameters["sigma"];
	const int dim = 3.0 * sigma + 1;
	const int n = 2 * dim + 1;
	ValueType sum = 0;
	ValueType u;

	ValueType x = -static_cast<ValueType>( dim );
	data::MemChunk<ValueType> kernel( n, 1, 1 );

	for( int i = 0; i < n; i++, x++ ) {
		u = xxgauss( x, sigma );
		sum += u;
		kernel.voxel<ValueType>( i, 0, 0 ) = u;
	}

	//normalize
	for( int i = 0; i < n; i++ ) {
		u = kernel.voxel<ValueType>( i, 0, 0 );
		u /= sum;
		kernel.voxel<ValueType>( i, 0, 0 ) = u;
	}

	output.reset( new data::Chunk( kernel ) );
	return true;
}

GaussianKernelFilter::ValueType GaussianKernelFilter::xxgauss ( const ValueType &x, const ValueType &sigma )
{
	const ValueType a = 2.506628273;
	const  ValueType z = x / sigma;
	return exp( static_cast<ValueType>( -z * z * 0.5 ) ) / ( sigma * a );
}


}
}