
#include <isis/data/io_interface.h>
#include <isis/data/io_factory.hpp>
#include <isis/util/tmpfile.hpp>
#include <isis/data/io_factory.hpp>

#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/filesystem/fstream.hpp>
#include <isis/data/fileptr.hpp>
#include <boost/iostreams/categories.hpp>  // tags


namespace isis
{
namespace image_io
{

class ImageFormat_Tar: public FileFormat{
	struct {
		char name[100];
		char mode[8];
		char uid[8];
		char gid[8];
		char size[12];
		char mtime[12];
		char chksum[8];
		char typeflag;
		char linkname[100];
		char magic[6];
		char version[2];
		char uname[32];
		char gname[32];
		char devmajor[8];
		char devminor[8];
		char prefix[155];
		char padding[12];
	} tar_header;
	
	bool read_header( const std::basic_istream<char> &src, size_t &size, size_t &next_header_in ) {
		if( boost::iostreams::read( src, reinterpret_cast<char *>( &tar_header ), 512 ) == 512 ) {
			if( tar_header.size[0] & 0x80 ) { // its base-256
				size = 0;

				for( uint_fast8_t i = 4; i < 11; i++ ) {
					size |= reinterpret_cast<uint8_t *>( tar_header.size )[i];
					size = size << 8;
				}

				size |= reinterpret_cast<uint8_t *>( tar_header.size )[11];
			} else if( tar_header.size[10] != 0 ) { //normal octal
				//get the size
				std::stringstream buff( tar_header.size );
				size = 0, next_header_in = 0;

				buff >> std::oct >> size;
			} else
				return false;

			next_header_in = ( size / 512 ) * 512 + ( size % 512 ? 512 : 0 );
			return true;
		} else
			return false;
	}
	static size_t tar_readstream( const std::basic_istream<char> &src, void *dst, size_t size, const std::string &log_title ) {
		size_t red = boost::iostreams::read( src, ( char * )dst, size ); // read data from the stream into the mapped memory

		if( red != size ) { // read the data from the stream
			LOG( Runtime, warning ) << "Could not read all " << size << " bytes for " << util::MSubject( log_title );
		}

		return red;
	}
protected:
	util::istring suffixes( io_modes modes )const override {return "tar";}
public:
	bool tainted()const override {return false;}
	std::string getName()const override {return "tar reading proxy";};
	void write( const data::Image &image, const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback )override {
		throwGenericError( "Not implemented (yet)" );
	}
	std::list<data::Chunk> load ( std::basic_streambuf<char> *source, std::list<util::istring> formatstack, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> progress )throw( std::runtime_error & ) override {
		std::list<data::Chunk> ret;
		size_t size, next_header_in;
		std::basic_istream<char> in(source);
		formatstack.pop_back(); //remove the "tar"

		while( in.good() && read_header( in, size, next_header_in ) ) { //read the header block

			boost::filesystem::path org_file;

			if( tar_header.typeflag == 'L' ) { // the filename of the next file is to long - so its stored in the next block (following this header)
				char namebuff[size];
				next_header_in -= tar_readstream( in, namebuff, size, "overlong filename for next entry" );
				in.ignore( next_header_in ); // skip the remaining input until the next header
				org_file = boost::filesystem::path( std::string( namebuff ) );
				LOG( Debug, verbose_info ) << "Got overlong name " << util::MSubject( org_file ) << " for next file.";

				read_header( in, size, next_header_in ); //continue with the next header
			} else {
				//get the original filename (use substr, because these fields are not \0-terminated)
				org_file = boost::filesystem::path( std::string( tar_header.prefix ).substr( 0, 155 ) + std::string( tar_header.name ).substr( 0, 100 ) );
			}

			if( size == 0 ) //if there is no content skip this entry (there are allways two "empty" blocks at the end of a tar)
				continue;

			if( tar_header.typeflag == '\0' || tar_header.typeflag == '0' ) { //only do regulars files

				
				data::IOFactory::FileFormatList formats = data::IOFactory::getFileFormatList( formatstack, dialect ); // try to get the reading plugin from the formatstack
				
				if(formats.empty()){ // if that fails try again with a formatstack from the filename
					formatstack=data::IOFactory::getFormatStack(org_file.native());
					formats= data::IOFactory::getFileFormatList( formatstack, dialect );
				}

				if( formats.empty() ) {
					LOG( Runtime, notice ) << "Skipping " << org_file << " inside the tar file because no plugin was found to read it"; // skip if we found none
					LOG( Runtime, notice ) << "You might want to define it with the \"-rf\" option (e.g. \"-rf dcm tar gz\" for dcm files inside a tar.gz)";
				} else {
					data::ValueArray<uint8_t> buffer(size);
					size_t red = boost::iostreams::read( in, std::static_pointer_cast<char>(buffer.getRawAddress()).get(), size ); // read data from the stream into the memory
					next_header_in -= red;

					if( red != size ) { // read the data from the stream
						LOG( Runtime, warning ) << "Could not read all " << size << " bytes for " << org_file;
					}

					// read the temporary file
					try {
						std::list<data::Chunk> loaded=data::IOFactory::loadChunks( buffer, formatstack, dialect.c_str() );
						for(data::Chunk &ref : loaded ) { // set the source property of the red chunks to something more usefull
							ref.setValueAs( "source", org_file.native() ); //@todo  add tar filename
						}
						ret.splice(ret.end(),loaded);
					} catch(data::IOFactory::io_error &e){
						LOG( Runtime, warning ) << "Failed to load " << org_file << " inside the tar file with " << e.which()->getName() << " (" << e.what() <<  " )"; // skip if we found none
					}
				}
			} else {
				LOG( Debug, verbose_info ) << "Skipping " << org_file << " inside the tar file because its no regular file (type is " << tar_header.typeflag << ")" ;
			}

			in.ignore( next_header_in ); // skip the remaining input until the next header
		}

		return ret;
	}
};

}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Tar();
}
