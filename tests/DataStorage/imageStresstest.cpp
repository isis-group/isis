#include "DataStorage/image.hpp"
#include <boost/timer.hpp>

using namespace isis;

const size_t slices = 128;
const size_t tsteps = 128;
const size_t slice_size = 128;

int main()
{
	boost::timer timer;
	data::Image img;
	timer.restart();
	unsigned short acq = 0;

	for ( size_t tstep = 0; tstep < tsteps; tstep++ ) {
		for ( size_t slice = 0; slice < slices; slice++ ) {
			data::MemChunk<short> chk( slice_size, slice_size );
			chk.setProperty( "readVec", util::fvector4( 1, 0 ) );
			chk.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
			chk.setProperty( "indexOrigin", util::fvector4( 0, 0, slice, tstep ) );
			chk.setProperty( "acquisitionNumber", ++acq );
			chk.setProperty( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );

			if ( !img.insertChunk( chk ) )
				std::cout << "Inserting Chunk " << slice << " failed" << std::endl;
		}
	}

	std::cout << tsteps << "*" << slices << " Chunks inserted in " << timer.elapsed() << " sec " << std::endl;
	timer.restart();
	img.reIndex();
	std::cout << "Image indexed in " << timer.elapsed() << " sec" << std::endl;
	timer.restart();

	for ( size_t tstep = 0; tstep < tsteps; tstep++ )
		for ( size_t slice = 0; slice < slices; slice++ )
			for ( size_t phase = 0; phase < slice_size; phase++ )
				for ( size_t read = 0; read < slice_size; read++ ) {
					short &ref = img.voxel<short>( read, phase, slice, tstep );
					ref = 42;
				}

	std::cout << tsteps *slices *slice_size *slice_size << " voxel set to 42 in " << timer.elapsed() << " sec" << std::endl;
	return 0;
}
