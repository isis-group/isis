
#include "DataStorage/io_interface.h"
#include <DataStorage/io_factory.hpp>
#include <CoreUtils/tmpfile.hpp>
#include <DataStorage/io_factory.hpp>

#define BOOST_FILESYSTEM_VERSION 2 //@todo switch to 3 as soon as we drop support for boost < 1.44
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include "DataStorage/fileptr.hpp"
#include <boost/iostreams/categories.hpp>  // tags

namespace isis
{
namespace image_io
{
namespace _internal
{

class progress_filter
{
public:
	progress_filter( util::ProgressFeedback &feedback ): m_feedback( feedback ), remain( 0 ) {}
	util::ProgressFeedback &m_feedback;
	std::streamsize remain;
	static const std::streamsize blocksize = 0x10000; //64k
	typedef char char_type;

	struct category : boost::iostreams::dual_use_filter_tag, boost::iostreams::multichar_tag { };

	void progress( std::streamsize n ) {
		remain += n;

		if( remain >= blocksize ) {
			m_feedback.progress( "", remain / blocksize );
			remain = remain % blocksize;
		}
	}
	template<typename Source>
	std::streamsize read( Source &src, char *s, std::streamsize n ) {
		progress( n );
		return boost::iostreams::read( src, s, n );
	}
	template<typename Sink>
	std::streamsize write( Sink &dest, const char *s, std::streamsize n ) {
		progress( n );;
		return boost::iostreams::write( dest, s, n );
	}
};
}

class ImageFormat_Compressed: public FileFormat
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
	static size_t tar_readstream( const boost::iostreams::filtering_istream &src, void *dst, size_t size, const std::string &log_title ) {
		size_t red = boost::iostreams::read( src, ( char * )dst, size ); // read data from the stream into the mapped memory

		if( red != size ) { // read the data from the stream
			LOG( Runtime, warning ) << "Could not read all " << size << " bytes for " << util::MSubject( log_title );
		}

		return red;
	}
protected:
	util::istring suffixes( io_modes modes = both )const {
		if( modes == write_only )
			return "gz bz2 Z";
		else
			return "tar tar.gz tgz tar.bz2 tbz tar.Z taz gz bz2 Z";
	}
public:
	util::istring dialects( const std::string &/*filename*/ )const {

		std::list<util::istring> suffixes;
		BOOST_FOREACH( data::IOFactory::FileFormatPtr format, data::IOFactory::getFormats() ) {
			const std::list<util::istring> s = format->getSuffixes();
			suffixes.insert( suffixes.end(), s.begin(), s.end() );
		}
		suffixes.sort();
		suffixes.unique();

		return util::listToString( suffixes.begin(), suffixes.end(), " ", "", "" ).c_str();
	}
	std::string getName()const {return "(de)compression proxy for other formats";}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> progress )
	throw( std::runtime_error & ) {
		std::list<data::Chunk>::iterator prev = chunks.end();
		--prev; //memory current position in the output list

		//select filters for input
		boost::iostreams::filtering_istream in;

		std::pair< std::string, std::string > proxyBase = makeBasename( filename );
		util::istring suffix = proxyBase.second.c_str();
		bool isTar = true;
		int ret = 0;

		if( suffix == ".tgz" )in.push( boost::iostreams::gzip_decompressor() ); // if its tgz,tbz or taz it IS tar
		else if( suffix == ".tbz" )in.push( boost::iostreams::bzip2_decompressor() );
		else if( suffix == ".taz" )in.push( boost::iostreams::zlib_decompressor() );
		else {
			if( suffix.find( ".tar" ) == 0 )suffix = suffix.substr( 4 ); // if there is an .tar*, as well
			else isTar = false; // else not

			// i case the input is compressed (tar.gz or .gz)
			if( isTar && suffix.empty() ); //if its tar having not compression is ok
			else if( suffix == ".gz" )in.push( boost::iostreams::gzip_decompressor() );
			else if( suffix == ".bz2" )in.push( boost::iostreams::bzip2_decompressor() );
			else if( suffix == ".Z" )in.push( boost::iostreams::zlib_decompressor() );
			else { // if its tar having no compression is fine
				throwGenericError( "Cannot determine the compression format of \"" + filename + "\"" );
			}
		}

		// add progress filter
		if( progress ) {
			progress->show( boost::filesystem::file_size( filename ) / _internal::progress_filter::blocksize, std::string( "decompressing " ) + filename );
			in.push( _internal::progress_filter( *progress ) );
		}

		// and on the top the source file
		std::ifstream input( filename.c_str(), std::ios_base::binary );
		input.exceptions( std::ios::badbit );
		in.push( input );

		if( isTar ) { // if it is tar we use out own tar "parser"
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

				if( tar_header.typeflag == '\0' || tar_header.typeflag == '0' ) { //only do regulars files

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

						size_t red = boost::iostreams::read( in, ( char * )&mfile[0], size ); // read data from the stream into the mapped memory
						next_header_in -= red;
						mfile.release(); //close and unmap the temporary file/mapped memory

						if( red != size ) { // read the data from the stream
							LOG( Runtime, warning ) << "Could not read all " << size << " bytes for " << tmpfile.file_string();
						}

						// read the temporary file
						std::list<data::Chunk>::iterator prev = chunks.end();
						--prev;
						ret += data::IOFactory::load( chunks, tmpfile.string(), dialect.c_str() );

						for( ; prev != chunks.end(); ++prev ) { // set the source property of the red chunks to something more usefull
							prev->setPropertyAs( "source", ( boost::filesystem::path( filename ) / org_file ).file_string() );
						}
					}
				} else {
					LOG( Debug, verbose_info ) << "Skipping " << org_file.file_string() << " because its no regular file (type is " << tar_header.typeflag << ")" ;
				}

				in.ignore( next_header_in ); // skip the remaining input until the next header
			}

		} else { // otherwise just decompress the file and read it
			const data::IOFactory::FileFormatList formats = data::IOFactory::getFileFormatList( proxyBase.first, dialect );

			if( formats.empty() ) {
				throwGenericError( "Cannot determine the uncompressed suffix of \"" + filename + "\" because no io-plugin was found for it" );
			}

			// set up the input stream
			util::TmpFile tmpFile( "", formats.front()->makeBasename( proxyBase.first ).second );
			std::ofstream output( tmpFile.file_string().c_str(), std::ios_base::binary );
			output.exceptions( std::ios::badbit );

			boost::iostreams::copy( in, output );

			ret = data::IOFactory::load( chunks, tmpFile.file_string().c_str(), dialect );

			if( ret ) { //re-set source of all new chunks
				prev++;
				LOG( Debug, info ) <<  "Setting source of all " << std::distance( prev, chunks.end() ) << " chunks to " << util::MSubject( filename );

				for( ; prev != chunks.end(); ++prev )prev->setPropertyAs( "source", filename );
			}
		}

		return ret;
	}

	void write( const data::Image &image, const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> progress )throw( std::runtime_error & ) {
		std::pair< std::string, std::string > proxyBase = makeBasename( filename );
		const util::istring suffix = proxyBase.second.c_str();

		const data::IOFactory::FileFormatList formats = data::IOFactory::getFileFormatList( proxyBase.first, dialect );

		if( formats.empty() ) {
			throwGenericError( "Cannot determine the uncompressed suffix of \"" + filename + "\" because no io-plugin was found for it" );
		}

		// create the intermediate file
		util::TmpFile tmpFile( "", formats.front()->makeBasename( proxyBase.first ).second );

		if( !data::IOFactory::write( image, tmpFile.file_string(), dialect ) ) {throwGenericError( tmpFile.file_string() + " failed to write" );}

		// set up the compression stream
		std::ifstream input( tmpFile.file_string().c_str(), std::ios_base::binary );
		std::ofstream output( filename.c_str(), std::ios_base::binary );
		input.exceptions( std::ios::badbit );
		output.exceptions( std::ios::badbit );

		boost::iostreams::filtering_ostream out;

		if( progress ) {
			progress->show( boost::filesystem::file_size( tmpFile ) / _internal::progress_filter::blocksize, std::string( "compressing " ) + filename );
			out.push( _internal::progress_filter( *progress ) );
		}

		if( suffix == ".gz" )out.push( boost::iostreams::gzip_compressor() );
		else if( suffix == ".bz2" )out.push( boost::iostreams::bzip2_compressor() );
		else if( suffix == ".Z" )out.push( boost::iostreams::zlib_compressor() );

		// write it
		out.push( output );
		boost::iostreams::copy( input, out );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Compressed();
}
