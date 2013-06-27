#include "DataStorage/image.hpp"
#include <boost/timer.hpp>

using namespace isis;

template<typename T> data::MemChunk<T> makeChunk( size_t size, short slice )
{
	data::MemChunk<T> ret( size, size );
	ret.setPropertyAs( "rowVec", util::fvector3( 1, 0 ) );
	ret.setPropertyAs( "columnVec", util::fvector3( 0, 1 ) );
	ret.setPropertyAs( "indexOrigin", util::fvector3( 0, 0, slice ) );
	ret.setPropertyAs( "voxelSize", util::fvector3( 1, 1, 1 ) );
	return ret;
}

int main()
{
	boost::timer timer;

	{
		std::cout << "===============Testing Chunk==================" << std::endl;
		std::list<data::Chunk> chunks;

		data::MemChunk<short> ch( 256, 256, 256 );
		timer.restart();

		BOOST_FOREACH( data::Chunk::reference ref, ch ) {
			ref = util::Value<int>( 42 );
		}

		std::cout << ch.getVolume() << " voxel set to 42 in " << timer.elapsed() << " sec" << std::endl;

		timer.restart();

		BOOST_FOREACH( data::Chunk::const_reference ref, ch ) {
			if( ref->as<int>() != 42 )
				std::cout << "Whoops, something is very wrong ..." << std::endl;
		}

		std::cout << ch.getVolume() << " voxel values red in " << timer.elapsed() << " sec" << std::endl;
	}
	{
		std::cout << "===============Testing Image==================" << std::endl;
		std::list<data::Chunk> chunks;
		const short slices = 256;
		timer.restart();

		for ( short slice = 0; slice < slices; slice++ ) {
			chunks.push_back( makeChunk<short>( 256, slice ) );
			chunks.back().setPropertyAs( "acquisitionNumber", slice );
			chunks.back().setPropertyAs( "sequenceNumber", 0 );
		}

		data::Image img( chunks );

		std::cout << slices << " Chunks inserted in " << timer.elapsed() << " sec " << std::endl;
		timer.restart();
		img.reIndex();
		std::cout << "Image indexed in " << timer.elapsed() << " sec, it has " << std::distance( img.begin(), img.end() )  << " Voxels" << std::endl;


		timer.restart();

		BOOST_FOREACH( data::Image::reference ref, img ) {
			ref = util::Value<int>( 42 );
		}

		std::cout << img.getVolume() << " voxel set to 42 in " << timer.elapsed() << " sec" << std::endl;
		timer.restart();

		BOOST_FOREACH( data::Image::const_reference ref, img ) {
			if( ref->as<int>() != 42 )
				std::cout << "Whoops, something is very wrong ..." << std::endl;
		}

		std::cout << img.getVolume() << " voxel values red in " << timer.elapsed() << " sec" << std::endl;
	}
	return 0;
}
