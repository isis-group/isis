#include <DataStorage/io_interface.h>
#include <fstream>
#include <boost/foreach.hpp>
#include "DataStorage/fileptr.hpp"
#include <boost/filesystem.hpp>

namespace isis
{
namespace image_io
{



class ImageFormat_raw: public FileFormat
{
	typedef std::map<std::string, unsigned short> typemap;
protected:
	std::string suffixes()const {
		return std::string( "raw" );
	}
public:
	std::string getName()const {
		return "raw data output";
	}
	virtual std::string dialects( const std::string & ) const {
		std::string ret;
		typemap types = util::getTransposedTypeMap( false, true );
		types.erase( "boolean*" );
		types.erase( "date*" );
		types.erase( "timestamp*" );
		types.erase( "selection*" );
		BOOST_FOREACH( typemap::const_reference pair, types ) {
			ret += pair.first.substr( 0, pair.first.length() - 1 );
			ret += " ";
		}
		return ret;
	}


	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {

		data::FilePtr mfile( filename );

		if( !mfile.good() ) {
			throwSystemError( errno, std::string( "Failed to open " ) + filename );
		}

		const size_t fsize = mfile.getLength();

		const unsigned short type = util::getTransposedTypeMap( false, true )[dialect+"*"];

		if( type == 0 ) {
			LOG( Runtime, error ) << "No known datatype given, you have to give the type of the raw data as rdialect (eg. \"-rdialect u16bit\")";
			throwGenericError( "No known datatype" );
		}


		data::ValuePtrReference dataRef = mfile.atByID( type, 0 );
		const size_t elemSize = dataRef->bytesPerElem();

		const size_t ssize = sqrt( fsize / elemSize );

		if( ssize *ssize *elemSize == fsize ) {
			LOG( Runtime, info ) << "Guessing size of read and phase to be " << ssize;

			chunks.push_back( data::Chunk( dataRef, ssize, ssize ) );
			data::Chunk &ch = chunks.back();
			ch.setPropertyAs<uint16_t>( "sequenceNumber", 0 );
			ch.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
			ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
			ch.setPropertyAs( "rowVec", util::fvector4( 1, 0 ) );
			ch.setPropertyAs( "columnVec", util::fvector4( 0, 1 ) );
			ch.setPropertyAs( "indexOrigin", util::fvector4( 0, 0 ) );
			return 1;
		} else {
			LOG( Runtime, error ) << "Could not guess image size for " << fsize << " bytes of data";
			return 0;
		}
	}

	void write( const data::Image &image, const std::string &filename, const std::string &/*dialect*/ )  throw( std::runtime_error & ) {
		class WriteOp: public data::ChunkOp
		{
			std::ofstream out;
			unsigned short typeID;
		public:
			WriteOp( std::string fname, unsigned short ID ): out( fname.c_str() ), typeID( ID ) {
				out.exceptions( std::ios::failbit | std::ios::badbit );
			}
			bool operator()( data::Chunk &ref, util::FixedVector<size_t, 4 > /*posInImage*/ ) {
				const boost::shared_ptr<const void> data( ref.getValuePtrBase().getRawAddress() );
				const size_t data_size = ref.bytesPerVoxel() * ref.getVolume();
				out.write( static_cast<const char *>( data.get() ), data_size );
				return true;
			}
		};
		const std::pair<std::string, std::string> splitted = makeBasename( filename );
		unsigned short type = image.getMajorTypeID();

		std::string typeStr = util::getTypeMap( false )[type];
		typeStr.erase( typeStr.find_last_not_of( '*' ) + 1 );

		const std::string outName = splitted.first + "_" + image.getSizeAsString() + "_" + typeStr + splitted.second ;

		LOG( ImageIoLog, info ) << "Writing image of size " << image.getSizeAsVector() << " and type " << typeStr << " to " << outName;
		WriteOp writer( outName, type );
		const_cast<data::Image &>( image ).foreachChunk( writer );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	isis::image_io::ImageFormat_raw *ret = new isis::image_io::ImageFormat_raw;
	return ret;
}
