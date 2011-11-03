#include <DataStorage/io_interface.h>

#include <time.h>

namespace isis
{
namespace image_io
{

class ImageFormat_Null: public FileFormat
{
	static const size_t timesteps = 20;
	std::list<data::Chunk> makeImage(unsigned short size,uint16_t sequence,std::string desc){
		//##################################################################################################
		//## standard null image
		//##################################################################################################
		std::list<data::Chunk> ret;
		for ( uint32_t t = 0; t < timesteps; t++ ) {
			for ( uint32_t s = 0; s < size; s++ ) {

				data::MemChunk<uint8_t> ch( size, size );
				ch.setPropertyAs( "indexOrigin", util::fvector4( 0, -150/2, s*110./size-100/2 ) );//don't use s*100./size-100/2 because we want a small gap
				ch.setPropertyAs<uint16_t>( "sequenceNumber", sequence );
				ch.setPropertyAs( "performingPhysician", std::string( "Dr. Jon Doe" ) );
				ch.setPropertyAs( "rowVec",    util::fvector4(  cos(M_PI/8), -sin(M_PI/8) ) ); //rotated by pi/8
				ch.setPropertyAs( "columnVec", util::fvector4(  sin(M_PI/8),  cos(M_PI/8) ) ); // @todo also rotate the sliceVec
				ch.setPropertyAs( "voxelSize", util::fvector4( 150./size, 150./size, 100./size ) );
				ch.setPropertyAs<uint16_t>( "repetitionTime", 1234 );
				ch.setPropertyAs( "sequenceDescription", desc );

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
	std::string suffixes(io_modes /*modes=both*/)const {
		return std::string( ".null .null.gz" );
	}
public:
	std::string getName()const {
		return "Null";
	}
	std::string dialects( const std::string &/*filename*/ )const {
		return std::string( "50 500 1000 2000" );
	}

	int load ( std::list<data::Chunk> &chunks, const std::string &/*filename*/, const std::string &dialect )  throw( std::runtime_error & ) {
		size_t size=10;
		if(!dialect.empty())
			size=boost::lexical_cast<unsigned short>(dialect);
		size=pow(size*1024*1024/timesteps,1./3.); //get qubic root for size volumes

		// normal sequencial image
		std::list<data::Chunk> ret=makeImage(size,0,"normal sequencial Image");
		uint32_t s=0;
		BOOST_FOREACH(data::Chunk &ref,ret){
			ref.setPropertyAs<uint32_t>( "acquisitionNumber", s++ );
		}
		chunks.insert(chunks.end(),ret.begin(),ret.end());

		// interleaved image
		ret=makeImage(size,1,"interleaved Image");
		std::list< data::Chunk >::iterator ch=ret.begin();
		for ( uint32_t t = 0; t < timesteps; t++ ) {
			//even numbers
			for ( uint32_t s = 0; s < (size/2.); s++ ) { //eg. size==5  2 < (5/2.) => 2 < 2.5 == true
				(ch++)->setPropertyAs<uint32_t>( "acquisitionNumber", s*2+t*size );
			}
			//uneven numbers
			for ( uint32_t s = 0; s < (size/2); s++ ) { //eg. size==5  2 < (5/2) => 2 < 2 == false
				(ch++)->setPropertyAs<uint32_t>( "acquisitionNumber", s*2+1+t*size );
			}
		}
		assert(ch==ret.end());
		chunks.insert(chunks.end(),ret.begin(),ret.end());
		
		return timesteps * size;
	}

	void write( const data::Image &image, const std::string &/*filename*/, const std::string &/*dialect*/ )  throw( std::runtime_error & ) {
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
