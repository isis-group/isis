#include <DataStorage/io_interface.h>

#include <time.h>

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
	std::string getName()const {
		return "Null";
	}
	std::string dialects( const std::string &/*filename*/ )const {
		return std::string( "memimage_50 memimage_500 memimage_1000 memimage_2000" );
	}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		if( dialect == "" ) {
			//##################################################################################################
			//## standard null image
			//##################################################################################################
			const size_t images = 5;
			const size_t timesteps = 10;

			for ( uint32_t i = 0; i < timesteps; i++ ) {
				for ( uint32_t c = 0; c < images; c++ ) {

					data::MemChunk<uint8_t> ch( 50, 50, 50 );
					ch.setPropertyAs( "indexOrigin", util::fvector4( 1, 2, 3 ) );
					ch.setPropertyAs<uint32_t>( "acquisitionNumber", i );
					ch.setPropertyAs<uint16_t>( "sequenceNumber", c );
					ch.setPropertyAs( "performingPhysician", std::string( "Dr. Jon Doe" ) );
					ch.setPropertyAs( "rowVec", util::fvector4(   ( float )1 / sqrtf( 2 ), ( float )1 / sqrtf( 2 ) ) ); //rotated by pi/4 (45°)
					ch.setPropertyAs( "columnVec", util::fvector4( ( float ) - 1 / sqrtf( 2 ), ( float )1 / sqrtf( 2 ) ) );
					ch.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
					ch.setPropertyAs( "voxelSize", util::fvector4( 1, 2, 3 ) );
					ch.setPropertyAs( "repetitionTime", 1234 );
					ch.setPropertyAs( "nullDescription", std::string( "The standard null image" ) );

					for ( int x = 10; x < 40; x++ )
						for ( int y = 10; y < 40; y++ )
							for ( int z = 0; z < 40; z++ )
								ch.voxel<uint8_t>( x, y, z ) = 255 - i * 20;

					ch.voxel<uint8_t>( 0, 0 ) = c * 40;
					chunks.push_back( ch );
				}
			}

			return timesteps * images;
		} else if ( dialect.find( "memimage_" ) == 0 ) {
			//##################################################################################################
			//## variable image
			//##################################################################################################
			typedef uint8_t IMAGE_TYPE;
			time_t secondsStart = std::time( NULL );
			std::stringstream sizeInMBStream( dialect.substr( 9, dialect.size() ) );
			size_t imageSizeMB;
			sizeInMBStream >> imageSizeMB;
			imageSizeMB *= ( 1024 * 1024 ) / sizeof( IMAGE_TYPE );
			size_t imageSize = pow( imageSizeMB, 1.0 / 4.0 );
			float voxelSize = 1.0;
			data::MemChunk<uint8_t> ch ( imageSize, imageSize, imageSize, imageSize );
			ch.setPropertyAs( "indexOrigin", util::fvector4() );
			ch.setPropertyAs<uint32_t>( "acquisitionNumber", 1 );
			ch.setPropertyAs<uint16_t>( "sequenceNumber", 1 );
			ch.setPropertyAs( "rowVec", util::fvector4( 1, 0, 0 ) );
			ch.setPropertyAs( "columnVec", util::fvector4( 0, 1, 0 ) );
			ch.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 )  );
			ch.setPropertyAs( "voxelGap", util::fvector4() );
			ch.setPropertyAs( "voxelSize", util::fvector4( voxelSize, voxelSize, voxelSize ) );
			std::stringstream imageSizeMBExact;
			imageSizeMBExact << ( float )( pow( imageSize, 4 ) * sizeof( IMAGE_TYPE ) ) / ( 1024 * 1024 ) << " mb";
			ch.setPropertyAs( "imageSize", imageSizeMBExact.str() );
			std::stringstream nullDescription;
			nullDescription << "Image with image size " << imageSize << "x" << imageSize << "x" << imageSize << "x" << imageSize;
			nullDescription << " and voxelSize " << voxelSize << "x" << voxelSize << "x" << voxelSize << "x" << voxelSize;
			ch.setPropertyAs( "nullDescription", nullDescription.str() );
			IMAGE_TYPE intensity;

			for ( size_t x = 0; x < imageSize; x++ ) {
				intensity = ( ( float )( std::numeric_limits<IMAGE_TYPE>::max() - std::numeric_limits<IMAGE_TYPE>::min() ) / imageSize ) * x;

				for ( size_t y = 0; y < imageSize; y++ ) {
					for ( size_t z = 0; z < imageSize; z++ ) {
						for ( size_t t = 0; t < imageSize ; t++ ) {
							ch.voxel<IMAGE_TYPE>( x, y, z, t ) = intensity;
						}
					}
				}
			}

			ch.setPropertyAs<size_t>( "timeUsedInS", ( ( float )( std::time( NULL ) - secondsStart ) ) ) ;
			chunks.push_back( ch );
			return 1;
		}
		return 0;
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
