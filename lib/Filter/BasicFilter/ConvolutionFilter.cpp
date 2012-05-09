#include "ConvolutionFilter.hpp"

namespace isis
{
namespace filter
{

bool ConvolutionFilter::process ( data::Chunk &chunk )
{
	const data::MemChunk<ValueType> kernel ( *m_additionalChunks.at( "kernel" ) );

	if( parameterMap.getPropertyAs<bool>( "convolveRow" ) ) {
		convolve( chunk, kernel, data::rowDim );
	}

	if( parameterMap.getPropertyAs<bool>( "convolveColumn" ) ) {
		convolve( chunk, kernel, data::columnDim );
	}

	if( parameterMap.getPropertyAs<bool>( "convolveSlice" ) ) {
		convolve( chunk, kernel, data::sliceDim );
	}

	return true;
}


void ConvolutionFilter::convolve ( data::Chunk &chunk, const data::Chunk &kernel, data::dimensions dim )
{
	util::ivector4 size = chunk.getSizeAsVector();
	util::ivector4 start;
	ValueType sum, x;
	int r0, r1, rr;
	util::ivector4 coords;
	const int d = kernel.getSizeAsVector()[0] / 2;
	size[dim] -= d;
	start[dim] = d;

	for( int32_t k = start[2]; k < size[2]; k++ ) {
		for( int32_t j = start[1]; j < size[1]; j++ ) {

			for( int32_t i = start[0]; i < size[0] - d; i++ ) {
				coords = util::ivector4( i, j, k );
				sum = 0;
				r0 = coords[dim] - d;
				r1 = coords[dim] + d;
				const ValueType *kp = &kernel.voxel<ValueType>( 0 );

				for ( rr = r0; rr <= r1; rr++ ) {
					coords[dim] = rr;
					x = chunk.voxel<ValueType>( coords[0], coords[1], coords[2] );
					sum += x * ( *kp++ );
				}

				chunk.voxel<ValueType>( i, j, k ) = sum;
			}
		}
	}
}



}
}