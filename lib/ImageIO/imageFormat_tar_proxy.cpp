
#include "DataStorage/io_interface.h"
#include <DataStorage/io_factory.hpp>
#include <CoreUtils/tmpfile.hpp>
#include <DataStorage/io_factory.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/lexical_cast.hpp>

#include <tar.h>
#include <fstream>
#include "DataStorage/fileptr.hpp"

namespace isis
{
namespace image_io
{

/**
 * IO-Proxy handling tar files.
 * \todo This might not work for OS with broken file handling (eg. Windows).
 */
class ImageFormat_TarProxy: public FileFormat
{
private:
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
	bool read_header( const boost::iostreams::filtering_istream &src, size_t &size, size_t &next_header_in ) {
		if( boost::iostreams::read( src, reinterpret_cast<char *>( &tar_header ), 512 ) == 512 ) {
			//get the size
			std::stringstream buff( tar_header.size );
			size = 0, next_header_in = 0;

			if( tar_header.size[10] != 0 ) {
				buff >> std::oct >> size;
				next_header_in = ( size / 512 ) * 512 + ( size % 512 ? 512 : 0 );
			}

			return true;
		} else
			return false;
	}
	static size_t tar_readstream( const boost::iostreams::filtering_istream &src, void *dst, size_t size, const std::string &log_title ) {
		size_t red = boost::iostreams::read( src, ( char * )dst, size ); // read data from the stream into the mapped memory

		if( red != size ) { // read the data from the stream
			LOG( Runtime, warning ) << "Could not read all " << size << " bytes for " << util::MSubject( log_title );
		}

		return red;
	}

protected:
	std::string suffixes()const {
		return std::string( "tar tar.gz tgz tar.bz2 tbz tar.Z taz" );
	}
public:
	std::string dialects( const std::string &/*filename*/ )const {

		std::list<util::istring> suffixes;
		BOOST_FOREACH( data::IOFactory::FileFormatPtr format, data::IOFactory::getFormats() ) {
			const std::list<util::istring> s = format->getSuffixes();
			suffixes.insert( suffixes.end(), s.begin(), s.end() );
		}
		suffixes.sort();
		suffixes.unique();

		return std::string( util::listToString( suffixes.begin(), suffixes.end(), " ", "", "" ) );
	}
	std::string getName()const {return "tar decompression proxy for other formats";}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) {
		int ret = 0;

		const util::istring suffix = makeBasename( filename ).second.c_str();


		// set up the input stream
		std::ifstream input( filename.c_str(), std::ios_base::binary );
		input.exceptions( std::ios::badbit );
		boost::iostreams::filtering_istream in;

		if( suffix == ".tar.gz" || suffix == ".tgz" )
			in.push( boost::iostreams::gzip_decompressor() );
		else if( suffix == ".tar.bz2" || suffix == ".tbz" )
			in.push( boost::iostreams::bzip2_decompressor() );
		else if( suffix == ".tar.Z" || suffix == ".taz" )
			in.push( boost::iostreams::zlib_decompressor() );

		in.push( input );

		size_t size, next_header_in;

		while( in.good() && read_header( in, size, next_header_in ) ) { //read the header block

			boost::filesystem::path org_file;

			if( tar_header.typeflag == 'L' ) { // the filename of the next file is to long - so its stored in the next block (following this header)
				char namebuff[size];
				next_header_in -= tar_readstream( in, namebuff, size, "overlong filename for next entry" );
				in.ignore( next_header_in ); // skip the remaining input until the next header
				org_file = boost::filesystem::path( namebuff );
				LOG( Debug, verbose_info ) << "Got overlong name " << util::MSubject( org_file.file_string() ) << " for next file.";

				read_header( in, size, next_header_in ); //continue with the next header
			} else {
				//get the original filename (use substr, because these fields are not \0-terminated)
				org_file = boost::filesystem::path( std::string( tar_header.prefix ).substr( 0, 155 ) + std::string( tar_header.name ).substr( 0, 100 ) );
			}

			if( size == 0 ) //if there is no content skip this entry (there are allways two "empty" blocks at the end of a tar)
				continue;

			if( tar_header.typeflag == AREGTYPE || tar_header.typeflag == REGTYPE ) {

				data::IOFactory::FileFormatList formats = data::IOFactory::getFileFormatList( org_file.file_string(), dialect ); // and get the reading pluging for that

				if( formats.empty() ) {
					LOG( Runtime, notice ) << "Skipping " << org_file << " from " << filename << " because no plugin was found to read it"; // skip if we found none
				} else {
					LOG( Debug, info ) << "Got " << org_file << " from " << filename << " there are " << formats.size() << " plugins which should be able to read it";

					const std::pair<std::string, std::string> base = formats.front()->makeBasename( org_file.file_string() );//ask any of the plugins for the suffix
					util::TmpFile tmpfile( "", base.second );//create a temporary file with this suffix

					data::FilePtr mfile( tmpfile, size, true );

					if( !mfile.good() ) {
						throwSystemError( errno, std::string( "Failed to open temporary " ) + tmpfile.file_string() );
					}

					LOG( Debug, info ) << "Mapped " << size << " bytes of " << mfile.getLength() << " at " << ( void * )&mfile[0];

					size_t red = boost::iostreams::read( in, ( char * )&mfile[0], size ); // read data from the stream into the mapped memory
					next_header_in -= red;
					mfile.close(); //close and unmap the temporary file/mapped memory

					if( red != size ) { // read the data from the stream
						LOG( Runtime, warning ) << "Could not read all " << size << " bytes for " << tmpfile.file_string();
					}

					// read the temporary file
					std::list<data::Chunk>::iterator prev = chunks.end();
					--prev;
					ret += data::IOFactory::load( chunks, tmpfile.string(), dialect );

					for( ; prev != chunks.end(); ++prev ) { // set the source property of the red chunks to something more usefull
						prev->setPropertyAs( "source", ( boost::filesystem::path( filename ) / org_file ).file_string() );
					}
				}
			} else {
				LOG( Debug, verbose_info ) << "Skipping " << org_file.file_string() << " because its no regular file (type is " << tar_header.typeflag << ")" ;
			}

			in.ignore( next_header_in ); // skip the remaining input until the next header
		}

		return ret;
	}

	void write( const data::Image &/*image*/, const std::string &/*filename*/, const std::string &/*dialect*/ )throw( std::runtime_error & ) {
		throw( std::runtime_error( "Writing to tar is not (yet) implemented" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_TarProxy();
}
