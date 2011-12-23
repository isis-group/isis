#ifdef _WINDOWS
#define ZLIB_WINAPI
#endif

#include "DataStorage/io_interface.h"
#include <DataStorage/io_factory.hpp>
#include <fstream>

namespace isis
{
namespace image_io
{

class ImageFormat_StdinProxy: public FileFormat
{
private:

protected:
	std::string suffixes( io_modes /*modes=both*/ )const {
		return std::string( "stdin" );
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
	std::string getName()const {return "stdin proxy (gets filenames from stdin)";}

	int load ( std::list<data::Chunk> &chunks, const std::string &/*filename*/, const std::string &dialect ) throw( std::runtime_error & ) {
		size_t red=0;
		while(!std::cin.eof()){
			std::string fnames; std::cin >> fnames ;
			BOOST_FOREACH(const std::string fname,util::stringToList<std::string>(fnames)){
				red += data::IOFactory::load( chunks, fname, "", dialect );
			}
		}
		return red;
	}

	void write( const data::Image &/*image*/, const std::string &/*filename*/, const std::string &/*dialect*/ )throw( std::runtime_error & ) {
		throw( std::runtime_error( "Compressed write is not yet implemented" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_StdinProxy();
}
