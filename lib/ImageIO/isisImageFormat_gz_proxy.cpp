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
	static void gz_compress( std::ifstream &in, gzFile out ) {
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

	static void file_compress( std::string infile, std::string outfile ) {
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

	static void gz_uncompress( gzFile in, std::ofstream &out ) {
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

	static void file_uncompress( std::string infile, std::string outfile ) {
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

protected:
	std::string suffixes()const {
		return std::string( ".gz" );
	}
public:
	std::string dialects( const std::string &filename )const {
		if( filename.empty() ) {
			return std::string();
		} else {
			std::set<std::string> ret;
			data::IOFactory::FileFormatList formats = data::IOFactory::get().getFormatInterface( FileFormat::makeBasename( filename ).first );
			BOOST_FOREACH( data::IOFactory::FileFormatList::const_reference ref, formats ) {
				const std::list<std::string> dias = util::string2list<std::string>( ref->dialects( filename ) );
				ret.insert( dias.begin(), dias.end() );
			}
			return util::list2string( ret.begin(), ret.end(), ",", "", "" );
		}
	}
	std::string name()const {return "compression proxy for other formats";}

	virtual std::pair<std::string, std::string> makeBasename( const std::string &filename )const {
		const std::pair<std::string, std::string> proxyBase = FileFormat::makeBasename( filename ); // get rid of the the .gz
		//then get the actual plugin for the format
		const data::IOFactory::FileFormatList formats = data::IOFactory::get().getFormatInterface( proxyBase.first );

		if( formats.empty() ) {
			LOG( Runtime, error ) << "Cannot determine the basename of " << util::MSubject( proxyBase.first ) << " because no io-plugin was found for it";
			return proxyBase;
		}

		//and ask it for the basename
		const std::pair<std::string, std::string> realBase = formats.front()->makeBasename( proxyBase.first );
		return std::make_pair( realBase.first, realBase.second + proxyBase.second );
	}

	int load ( data::ChunkList &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) {
		const std::pair<std::string, std::string> proxyBase = FileFormat::makeBasename( filename ); // get rid of the the .gz
		//then get the actual plugin for the format
		const data::IOFactory::FileFormatList formats = data::IOFactory::get().getFormatInterface( proxyBase.first );

		if( formats.empty() ) {
			throwGenericError( "Cannot determine the unzipped suffix of \"" + filename + "\" because no io-plugin was found for it" );
		}

		const std::pair<std::string, std::string> realBase = formats.front()->makeBasename( proxyBase.first );

		util::TmpFile tmpfile( "", realBase.second );

		LOG( Debug, info ) <<  "tmpfile=" << tmpfile;

		file_uncompress( filename, tmpfile.string() );

		data::ChunkList buff;

		int ret = data::IOFactory::get().loadFile( buff, tmpfile, "", dialect );

		if( ret ) {
			LOG( Debug, info ) <<  "Setting source of all " << buff.size() << " chunks to " << util::MSubject( filename );
			BOOST_FOREACH( data::ChunkList::reference ref, buff ) {
				ref->setProperty( "source", filename );
			}
			chunks.insert( chunks.end(), buff.begin(), buff.end() );
		}
	}

	void write( const data::Image &image, const std::string &filename, const std::string &dialect )throw( std::runtime_error & ) {
		throw( std::runtime_error( "Compressed write is not yet implemented" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_CompProxy();
}
