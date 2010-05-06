#include "DataStorage/io_interface.h"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/tmpfile.h"
#include <stdio.h>
#include <fstream>
#include <zlib.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

namespace isis
{
namespace image_io
{

class ImageFormat_CompProxy: public FileFormat
{
private:
	void gz_compress( std::ifstream &in, gzFile out ) {
		char buf[2048*1024];
		int len;

		for (
			in.read( buf, 2048 * 1024 );
			( len = in.gcount() );
			in.read( buf, 2048 * 1024 )
		) {
			if ( gzwrite( out, buf, len ) != len ) {
				int err;
				gzerror( out, &err );

				// If an error occurred in the file system and not in the compression library, err is set to Z_ERRNO
				if ( err == Z_ERRNO ) {
					throwSystemError( errno, "Failed to compress file" );
				} else {
					throwGenericError( "Failed to compress file" );
				}
			}
		}
	}

	void file_compress( std::string infile, std::string outfile ) {
		std::ifstream in;
		in.exceptions( std::ifstream::failbit | std::ifstream::badbit );
		in.open( infile.c_str(), std::ios::binary );
		gzFile out = gzopen( outfile.c_str(), "wb" );

		if ( out == NULL ) {
			if ( errno )
				throwSystemError( errno );
			else //zlib says, that if errno is zero there was insufficient memory
				throwGenericError( "insufficient memory for compression" );
		}

		gz_compress( in, out );

		if ( gzclose( out ) != Z_OK ) {
			LOG( ImageIoLog, warning ) << "gzclose " << outfile << " failed";
		}
	}

	void gz_uncompress( gzFile in, std::ofstream &out ) {
		char buf[2048*1024];
		int len;

		for (
			len = gzread( in, buf, 2048 * 1024 );
			len;
			len = gzread( in, buf, 2048 * 1024 )
		) {
			if ( len < 0 ) {
				int err;
				gzerror( out, &err );

				// If an error occurred in the file system and not in the compression library, err is set to Z_ERRNO
				if ( err == Z_ERRNO ) {
					throwSystemError( errno, "Failed to read compressed file" );
				} else {
					throwGenericError( "Failed to read compressed file" );
				}
			} else {
				out.write( buf, len );
			}
		}
	}

	void file_uncompress( std::string infile, std::string outfile ) {
		gzFile in = gzopen( infile.c_str(), "rb" );

		if ( in == NULL ) {
			if ( errno )
				throwSystemError( errno );
			else //zlib says, that if errno is zero there was insufficient memory
				throwGenericError( "insufficient memory for compression" );
		}

		std::ofstream out;
		out.exceptions( std::ifstream::failbit | std::ifstream::badbit );
		out.open( outfile.c_str(), std::ios::binary );
		gz_uncompress( in, out );

		if ( gzclose( in ) != Z_OK ) {
			LOG( ImageIoLog, warning ) << "gclose " << outfile << " failed";
		}
	}


public:
	std::string suffixes() {
		return std::string( ".gz" );
	}
	/*  std::string dialects(){
	        return std::string("inverted");
	    }*/
	std::string name() {return "compression proxy for other formats";}

	int load ( data::ChunkList &chunks, const std::string& filename, const std::string& dialect ) throw( std::runtime_error& ) {
		const std::string unzipped_suffix = boost::filesystem::extension( boost::filesystem::basename( filename ) );
		util::TmpFile tmpfile( "", unzipped_suffix );
		LOG( ImageIoDebug, info ) <<  "tmpfile=" << tmpfile;
		file_uncompress( filename, tmpfile.string() );
		return data::IOFactory::get().loadFile( chunks, tmpfile, dialect );
	}

	void write( const data::Image &image, const std::string& filename, const std::string& dialect )throw( std::runtime_error& ) {
		throw( std::runtime_error( "Compressed write is not yet implemented" ) );
	}
	bool tainted() {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat* factory()
{
	return new isis::image_io::ImageFormat_CompProxy();
}
