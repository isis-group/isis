#include "DataStorage/image.hpp"
#include <boost/timer.hpp>

using namespace isis;

const size_t slices = 128;
const size_t tsteps = 128;
const size_t slice_size = 128;

int main()
{
	boost::timer timer;
	std::list<data::Chunk> chunks;
	timer.restart();
	uint32_t acq = 0;

	for ( size_t tstep = 0; tstep < tsteps; tstep++ ) {
		for ( size_t slice = 0; slice < slices; slice++ ) {
			chunks.push_back(data::MemChunk<short>( slice_size, slice_size ));
			chunks.back().setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			chunks.back().setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			chunks.back().setPropertyAs( "indexOrigin", util::fvector4( 0, 0, slice ) );
			chunks.back().setPropertyAs( "acquisitionNumber", ++acq );
			chunks.back().setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
		}
	}

	data::Image img(chunks);

	std::cout << tsteps << "*" << slices << " Chunks inserted in " << timer.elapsed() << " sec " << std::endl;
	timer.restart();
	img.reIndex();
	std::cout << "Image indexed in " << timer.elapsed() << " sec" << std::endl;
	timer.restart();

	for ( size_t tstep = 0; tstep < tsteps; tstep++ )
		for ( size_t slice = 0; slice < slices; slice++ )
			for ( size_t column = 0; column < slice_size; column++ )
				for ( size_t row = 0; row < slice_size; row++ ) {
					short &ref = img.voxel<short>( row, column, slice, tstep );
					ref = 42;
				}

	std::cout << tsteps *slices *slice_size *slice_size << " voxel set to 42 in " << timer.elapsed() << " sec" << std::endl;
	return 0;
}
