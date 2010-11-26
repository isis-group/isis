#include "DataStorage/io_interface.h"

namespace isis
{
namespace image_io
{

class ImageFormat_Null: public FileFormat
{
protected:
	std::string suffixes()const {
		return std::string( ".null .null.gz" );
	}
public:
	std::string name()const {
		return "Null";
	}

	int load ( data::ChunkList &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		const size_t images = 5;
		const size_t timesteps = 10;

		for ( size_t i = 0; i < timesteps; i++ ) {
			for ( size_t c = 0; c < images; c++ ) {
				boost::shared_ptr<data::Chunk > ch( new data::MemChunk<uint8_t>( 50, 50, 50 ) );
				ch->setPropertyAs( "indexOrigin", util::fvector4( 1, 2, 3 ) );
				ch->setPropertyAs<uint32_t>( "acquisitionNumber", i );
				ch->setPropertyAs<uint16_t>( "sequenceNumber", c );
				ch->setPropertyAs( "performingPhysician", std::string( "Dr. Jon Doe" ) );
				ch->setPropertyAs( "readVec", util::fvector4(   ( float )1 / sqrtf( 2 ), ( float )1 / sqrtf( 2 ) ) ); //rotated by pi/4 (45°)
				ch->setPropertyAs( "phaseVec", util::fvector4( ( float ) - 1 / sqrtf( 2 ), ( float )1 / sqrtf( 2 ) ) );
				ch->setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
				ch->setPropertyAs( "voxelSize", util::fvector4( 1, 2, 3 ) );
				ch->setPropertyAs( "repetitionTime", 1234 );

				for ( int x = 10; x < 40; x++ )
					for ( int y = 10; y < 40; y++ )
						for ( int z = 0; z < 40; z++ )
							ch->voxel<uint8_t>( x, y, z ) = 255 - i * 20;

				ch->voxel<uint8_t>( 0, 0 ) = c * 40;
				chunks.push_back( ch );
			}
		}

		return timesteps * images; //return data::ChunkList();
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		if ( image.getSizeAsVector() != util::fvector4( 50, 50, 50, 10 ) )
			throw( std::runtime_error( "Size mismatch (" + image.getSizeAsString() + "!=" + boost::lexical_cast<std::string>( util::fvector4( 50, 50, 50, 10 ) ) + ")" ) );

		const int snum = image.getPropertyAs<int32_t>( "sequenceNumber" );

		for ( int i = 0; i < 10; i++ )
			if ( image.voxel<uint8_t>( 0, 0 ) != snum )
				throw( std::runtime_error( "Data mismatch" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Null();
}
