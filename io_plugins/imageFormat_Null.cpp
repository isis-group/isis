#include <data/io_interface.h>

#include <time.h>

namespace isis
{
namespace image_io
{

class ImageFormat_Null: public FileFormat
{
	static const size_t timesteps = 20;
	std::list<data::Chunk> makeImage( unsigned short size, uint16_t sequence, std::string desc ) {
		//##################################################################################################
		//## standard null image
		//##################################################################################################
		std::list<data::Chunk> ret;

		for ( uint32_t t = 0; t < timesteps; t++ ) {
			for ( uint32_t s = 0; s < size; s++ ) {

				data::MemChunk<uint8_t> ch( size, size );
				ch.setValueAs( "indexOrigin", util::fvector3( 0, -150 / 2, s * 110. / size - 100 / 2 ) ); //don't use s*100./size-100/2 because we want a small gap
				ch.setValueAs<uint16_t>( "sequenceNumber", sequence );
				ch.setValueAs( "performingPhysician", std::string( "Dr. Jon Doe" ) );
				ch.setValueAs( "rowVec",    util::fvector3(  cos( M_PI / 8 ), -sin( M_PI / 8 ) ) ); //rotated by pi/8
				ch.setValueAs( "columnVec", util::fvector3(  sin( M_PI / 8 ),  cos( M_PI / 8 ) ) ); // @todo also rotate the sliceVec
				ch.setValueAs( "voxelSize", util::fvector3( 150. / size, 150. / size, 100. / size ) );
				ch.setValueAs<uint16_t>( "repetitionTime", 1234 );
				ch.setValueAs( "sequenceDescription", desc );

				for ( int x = 10; x < 40; x++ )
					for ( int y = 10; y < 40; y++ )
						ch.voxel<uint8_t>( x, y ) = 255 - s * 20;

				ch.voxel<uint8_t>( 0, 0 ) = t * 40;
				ret.push_back( ch );
			}
		}

		return ret;
	}
protected:
	util::istring suffixes( io_modes /*modes=both*/ )const {return ".null"; }
	size_t getSize( const util::istring &dialect ) {
		size_t size = 10;

		if( !dialect.empty() )
			size = boost::lexical_cast<unsigned short>( dialect );

		size = pow( size * 1024 * 1024 / timesteps, 1. / 3. ); //get qubic root for size volumes
		return size;
	}
public:
	std::string getName()const {
		return "Null";
	}
	util::istring dialects( const std::string &/*filename*/ )const {
		return "50 500 1000 2000";
	}

	std::list<data::Chunk> load ( const std::string &/*filename*/, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> /*progress*/ )  throw( std::runtime_error & ) {

		size_t size = getSize( dialect );

		// normal sequencial image
		std::list<data::Chunk> ret,loaded = makeImage( size, 0, "normal sequencial Image" );
		uint32_t s = 0;
		for( data::Chunk & ref :  ret ) {
			ref.setValueAs<uint32_t>( "acquisitionNumber", s++ );
		}
		ret.splice( ret.end(), loaded );

		// interleaved image
		loaded= makeImage( size, 1, "interleaved Image" );
		std::list< data::Chunk >::iterator ch = loaded.begin();

		for ( size_t t = 0; t < timesteps; t++ ) {
			//even numbers
			for ( uint32_t a = 0; a < ( size / 2. ); a++ ) { //eg. size==5  2 < (5/2.) => 2 < 2.5 == true
				( ch++ )->setValueAs<uint32_t>( "acquisitionNumber", a * 2 + t * size );
			}

			//uneven numbers
			for ( uint32_t a = 0; a < ( size / 2 ); a++ ) { //eg. size==5  2 < (5/2) => 2 < 2 == false
				( ch++ )->setValueAs<uint32_t>( "acquisitionNumber", a * 2 + 1 + t * size );
			}
		}

		assert( ch == loaded.end() );
		ret.splice( ret.end(), loaded );

		return timesteps * size;
	}

	void write( const data::Image &img, const std::string &/*filename*/, const util::istring &/*dialect*/, std::shared_ptr<util::ProgressFeedback> /*progress*/ )  throw( std::runtime_error & ) {
		data::Image image = img;

		// set by the core, thus the newChunks cannot have one
		image.remove( "source" );
		image.remove( "voxelGap" );
		image.remove( "sliceVec" );

		size_t size = image.getSizeAsVector()[0];
		std::list< data::Chunk > newChunks;
		std::vector< data::Chunk > oldChunks = image.copyChunksToVector();
		std::list< data::Chunk >::iterator iCh;
		uint32_t s = 0;

		switch( image.getValueAs<int>( "sequenceNumber" ) ) {
		case 0: //image 0 is a "normal" image
			newChunks = makeImage( size, 0, "normal sequencial Image" );
			for( data::Chunk & ref :  newChunks ) {
				ref.setValueAs<uint32_t>( "acquisitionNumber", s++ );
			}
			break;
		case 1: //image 1 is a "interleaved" image
			newChunks = makeImage( size, 1, "interleaved Image" );
			iCh = newChunks.begin();

			for ( uint32_t t = 0; t < timesteps; t++ ) {
				//even numbers
				for ( uint32_t s = 0; s < ( size / 2. ); s++ ) { //eg. size==5  2 < (5/2.) => 2 < 2.5 == true
					( iCh++ )->setValueAs<uint32_t>( "acquisitionNumber", s * 2 + t * size );
				}

				//uneven numbers
				for ( uint32_t s = 0; s < ( size / 2 ); s++ ) { //eg. size==5  2 < (5/2) => 2 < 2 == false
					( iCh++ )->setValueAs<uint32_t>( "acquisitionNumber", s * 2 + 1 + t * size );
				}
			}

			assert( iCh == newChunks.end() );
			break;
		default:
			throwGenericError( "unknown Image" );
		}

		if( newChunks.size() != oldChunks.size() )
			throwGenericError( "ammount of chunks differs" );

		std::list< data::Chunk >::iterator newCH = newChunks.begin();

		for( size_t i = 0; i < oldChunks.size(); ++i, ++newCH ) {
			// check for the orientation seperately
			if(
				newCH->getValueAs<util::fvector3>( "columnVec" ).fuzzyEqual( oldChunks[i].getValueAs<util::fvector3>( "columnVec" ) ) == false ||
				newCH->getValueAs<util::fvector3>( "rowVec" ).fuzzyEqual( oldChunks[i].getValueAs<util::fvector3>( "rowVec" ) ) == false
			) {
				throwGenericError( "orientation is not equal" );
			} else {
				newCH->remove( "rowVec" );
				newCH->remove( "columnVec" );
				oldChunks[i].remove( "rowVec" );
				oldChunks[i].remove( "columnVec" );
			}

			util::PropertyMap::DiffMap metaDiff = newCH->getDifference( oldChunks[i] );

			if( metaDiff.size() ) {
				std::cerr << metaDiff << std::endl;
				throwGenericError( "differences in the metainformation found" );
			}

			if( newCH->compare( oldChunks[i] ) ) {
				throwGenericError( "voxels do not fit" );
			}
		}
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Null();
}
