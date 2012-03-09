#ifdef _WINDOWS
#define ZLIB_WINAPI
#endif

#include "DataStorage/io_interface.h"
#include <DataStorage/io_factory.hpp>
#include <stdio.h>
#include <errno.h>

namespace isis
{
namespace image_io
{

class ImageFormat_StdinProxy: public FileFormat
{
private:

protected:
	util::istring suffixes( io_modes /*modes=both*/ )const {return "process";}
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
	std::string getName()const {return "process proxy (gets filenames from child process given in the filename)";}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const util::istring &dialect ) throw( std::runtime_error & ) {
		size_t red = 0;
		LOG( Runtime, info ) << "Running " << util::MSubject( filename );
		FILE *in = popen( filename.c_str(), "r" );

		if( in == NULL ) {
			std::string err = "Failed to run \"";
			err += filename + "\"";
			throwSystemError( errno, err );
		}

		char got;
		std::string fname;
		size_t fcnt = 0;

		while( ( got = fgetc( in ) ) != EOF ) {
			if( got == '\n' ) {
				if( !fname.empty() ) {
					LOG( Runtime, info ) << "Got " << util::MSubject( fname ) << " from " << util::MSubject( filename );
					red += data::IOFactory::load( chunks, fname, dialect.c_str(), "" );
					fname.clear();
					fcnt++;
				}
			} else {
				fname += got;
			}
		}

		pclose( in );
		LOG_IF( fcnt == 0, Runtime, warning ) << "didn't get any filename from " << util::MSubject( filename );
		return red;
	}

	void write( const data::Image &/*image*/, const std::string &/*filename*/, const util::istring &/*dialect*/ )throw( std::runtime_error & ) {
		throw( std::runtime_error( "not yet implemented" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_StdinProxy();
}
