#include <DataStorage/io_interface.h>
#include <fstream>
#include <boost/foreach.hpp>

namespace isis
{
namespace image_io
{

class ImageFormat_raw: public FileFormat
{
protected:
	std::string suffixes()const {
		return std::string( "raw" );
	}
public:
	std::string getName()const {
		return "raw data output";
	}


	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		throwGenericError( "Sorry raw input is not supported yet" );
		return 0;
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )  throw( std::runtime_error & ) {
		const std::pair<std::string, std::string> splitted = makeBasename( filename );
		std::string type = image.getMajorTypeName();
		type = type.substr( 0, type.find_last_not_of( '*' ) + 1 );
		const std::string outName = splitted.first + "_" + image.getSizeAsString() + "_" + type + splitted.second ;

		LOG( ImageIoLog, info ) << "Writing image of size " << image.getSizeAsVector() << " and type " << type << " to " << outName;
		std::ofstream out( outName.c_str() );
		out.exceptions( std::ios::failbit | std::ios::badbit );
		BOOST_FOREACH( const boost::shared_ptr<const data::Chunk > &ref, image.getChunksAsVector() ) {
			const boost::shared_ptr<void> data( ref->getValuePtrBase().getRawAddress() );
			const size_t data_size = ref->bytesPerVoxel() * ref->getVolume();
			out.write( static_cast<const char *>( data.get() ), data_size );
		}
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_raw();
}
