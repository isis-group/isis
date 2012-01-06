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

class ImageFormat_FListProxy: public FileFormat
{
private:

protected:
	std::string suffixes( io_modes /*modes=both*/ )const {
		return std::string( "flist" );
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
	std::string getName()const {return "filelist proxy (gets filenames from files or stdin)";}

	size_t doLoad(std::istream &in,std::list<data::Chunk> &chunks, const std::string &dialect){
		size_t red=0;
		const boost::regex linebreak("[[.newline.][.carriage-return.]]");
		std::string fnames;
		while(!in.eof()){
			in >> fnames ;
			BOOST_FOREACH(const std::string fname,util::stringToList<std::string>(fnames,linebreak)){
				LOG(Runtime,info) << "loading " << fname; 
				red += data::IOFactory::load( chunks, fname, "", dialect );
			}
		}
		return red;
	}

	int load ( std::list<data::Chunk> &chunks, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & ) {
		if(filename.empty()){
			LOG(Runtime,info) << "Getting filelist from stdin";
			return doLoad(std::cin,chunks,dialect);
		} else {
			LOG(Runtime,info) << "Getting filelist from " << filename;
			std::ifstream in(filename.c_str());
			in.exceptions( std::ios::badbit );
			return doLoad(in,chunks,dialect);
		}
	}

	void write( const data::Image &/*image*/, const std::string &/*filename*/, const std::string &/*dialect*/ )throw( std::runtime_error & ) {
		throw( std::runtime_error( "not yet implemented" ) );
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_FListProxy();
}
