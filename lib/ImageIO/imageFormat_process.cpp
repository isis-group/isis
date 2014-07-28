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
			std::list<util::istring> s = format->getSuffixes();
			suffixes.splice( suffixes.end(), s );
		}
		suffixes.sort();
		suffixes.unique();

		return util::listToString( suffixes.begin(), suffixes.end(), " ", "", "" ).c_str();
	}
	std::string getName()const {return "process proxy (gets filenames from child process given in the filename)";}

	std::list<data::Chunk> load ( const std::string &filename, const util::istring &dialect, boost::shared_ptr<util::ProgressFeedback> /*progress*/ ) throw( std::runtime_error & ) {
		
		LOG( Runtime, info ) << "Running " << util::MSubject( filename );
		FILE *in = popen( filename.c_str(), "r" );
		std::list< data::Chunk > ret;

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
					std::list< data::Chunk > loaded=data::IOFactory::loadChunks( fname, dialect.c_str(), "" );
					ret.splice(ret.end(),loaded);
					fname.clear();
					fcnt++;
				}
			} else {
				fname += got;
			}
		}

		pclose( in );
		LOG_IF( fcnt == 0, Runtime, warning ) << "didn't get any filename from " << util::MSubject( filename );
		return ret;
	}

	void write( const data::Image &/*image*/, const std::string &/*filename*/, const util::istring &/*dialect*/, boost::shared_ptr<util::ProgressFeedback> /*progress*/ )throw( std::runtime_error & ) {
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
