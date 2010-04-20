#include <DataStorage/io_interface.h>

namespace isis
{
namespace image_io
{

class ImageFormat_Null: public FileFormat
{
public:
	std::string suffixes() {
		return std::string( ".null .null.gz" );
	}
	std::string dialects() {
		return std::string( "inverted" );
	}
	std::string name() {
		return "Null";
	}

	int load ( data::ChunkList &chunks, const std::string& filename, const std::string& dialect ) {
		const size_t images = 5;
		const size_t timesteps = 10;

		for ( int i = 0; i < timesteps; i++ ) {
			for ( int c = 0; c < images; c++ ) {
				data::MemChunk<u_int8_t> ch( 50, 50, 50 );
				ch.setProperty( "indexOrigin", util::fvector4( 0, 0, 0, i ) );
				ch.setProperty( "acquisitionNumber", c );
				ch.setProperty( "sequenceNumber", c );
				ch.setProperty( "performingPhysician", std::string( "Dr. Jon Doe" ) );
				ch.setProperty( "readVec", util::fvector4( 1, 0 ) );
				ch.setProperty( "phaseVec", util::fvector4( 0, 1 ) );
				ch.setProperty( "voxelSize", util::fvector4( 1, 1, 1 ) );
				ch.voxel<u_int8_t>( 0, 0 ) = c;

				for ( int x = 10; x < 40; x++ )
					for ( int y = 10; y < 40; y++ )
						for ( int z = 10; z < 40; z++ )
							ch.voxel<u_int8_t>( x, y, z ) = 255 - i * 20;

				chunks.push_back( ch );
			}
		}

		return timesteps*images;//return data::ChunkList();
	}

	bool write( const data::Image &image, const std::string& filename, const std::string& dialect ) {
		if ( image.sizeToVector() != util::fvector4( 50, 50, 50, 10 ) )return false;

		const int snum = image.getProperty<int>( "sequenceNumber" );

		for ( int i = 0; i < 10; i++ )
			if ( image.voxel<u_int8_t>( 0, 0 ) != snum )return false;

		return true;
	}
	bool tainted() {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat* factory()
{
	return new isis::image_io::ImageFormat_Null();
}
