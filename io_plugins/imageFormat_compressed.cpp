
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

#ifdef HAVE_LZMA
#include "imageFormat_compressed_lzma.hpp"
#endif //HAVE_LZMA

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
	static const std::streamsize blocksize = 0x40000; //256k
	typedef char char_type;

	struct category : boost::iostreams::dual_use_filter_tag, boost::iostreams::multichar_tag { };

	void progress( std::streamsize n ) {
		m_feedback.progress( "", n );
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
protected:
	util::istring suffixes( io_modes modes = both )const override {
		static util::istring write="gz bz2 Z xz";
#ifdef HAVE_LZMA
		write+=" xz";
#endif //HAVE_LZMA
		if( modes == write_only )return write;
		else return write+" tgz tbz taz";
	}
public:
	std::unique_ptr<boost::iostreams::filtering_istream> makeIStream(std::list<util::istring> &formatstack){
		//select filters for input
		std::unique_ptr<boost::iostreams::filtering_istream> in(new boost::iostreams::filtering_istream);

		util::istring format = formatstack.back();
		formatstack.back()="tar"; // push in "tar" in case its one of the following

		if( format == "tgz" )in->push( boost::iostreams::gzip_decompressor() ); // if its tgz,tbz or taz it IS tar
		else if( format == "tbz" )in->push( boost::iostreams::bzip2_decompressor() );
		else if( format == "taz" )in->push( boost::iostreams::zlib_decompressor() );
		else {
			formatstack.pop_back(); // ok, its none of the above, so remove the additional "tar"
			if( format == "gz" )in->push( boost::iostreams::gzip_decompressor() );
			else if( format == "bz2" )in->push( boost::iostreams::bzip2_decompressor() );
			else if( format == "Z" )in->push( boost::iostreams::zlib_decompressor() );
#ifdef HAVE_LZMA
			else if( format == "xz" )in->push( boost::iostreams::lzma_decompressor() );
#endif
			else { // ok, no idea what going on, cry for mamy
				throwGenericError( "Cannot determine the compression format" );
			}
		}
		return std::move(in);
	}
	std::string getName()const override {return "(de)compression proxy for other formats";}

	std::list<data::Chunk> load ( std::basic_streambuf<char> *source, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> progress )throw( std::runtime_error & ) override {
		
		auto in=makeIStream(formatstack);

		//ok, we have a filter, use that with the source stream on top
		in->push( *source );

		return data::IOFactory::loadChunks( in->rdbuf(), formatstack, dialects );
	}
	std::list<data::Chunk> load( const boost::filesystem::path &filename, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & ) override{
		//try open file
		std::ifstream file(filename.c_str());
		file.exceptions(std::ios_base::badbit);

		// set up progress bar if its enabled but don't fiddle with it if its set up already
		bool set_up=false;
		if( feedback && feedback->getMax() == 0 ) {
			set_up=true;
			feedback->show( boost::filesystem::file_size( filename ), std::string( "decompressing " ) + filename.native() );
		}
		auto in=makeIStream(formatstack);

		if(set_up) 
			in->push( _internal::progress_filter( *feedback ) );

		in->push(file);
		std::list<data::Chunk> ret=data::IOFactory::loadChunks( in->rdbuf(), formatstack, dialects );
		
		if(set_up) // close progress bar
			feedback->close();
		return ret;
	}

	void write( const data::Image &image, const std::string &filename, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> progress ) override {
		std::pair< std::string, std::string > proxyBase = makeBasename( filename );
		const util::istring suffix = proxyBase.second.c_str();

		const auto formatstack = data::IOFactory::getFormatStack(proxyBase.first);
		const data::IOFactory::FileFormatList formats = data::IOFactory::getFileFormatList( formatstack );

		if( formats.empty() ) {
			throwGenericError( "Cannot determine the uncompressed suffix of \"" + filename + "\" because no io-plugin was found for it" );
		}

		// create the intermediate file
		util::TmpFile tmpFile( "", formats.front()->makeBasename( proxyBase.first ).second );

		if( !data::IOFactory::write( image, tmpFile.native(), formatstack, dialects ) ) {throwGenericError( tmpFile.native() + " failed to write" );}

		// set up the compression stream
		boost::filesystem::ifstream input( tmpFile, std::ios_base::binary );
		std::ofstream output( filename.c_str(), std::ios_base::binary );
		input.exceptions( std::ios::badbit );
		output.exceptions( std::ios::badbit );

		boost::iostreams::filtering_ostream out;

		if( progress ) {
			progress->show( boost::filesystem::file_size( tmpFile ), std::string( "compressing " ) + filename );
			out.push( _internal::progress_filter( *progress ) );
		}

		if( suffix == ".gz" )out.push( boost::iostreams::gzip_compressor() );
		else if( suffix == ".bz2" )out.push( boost::iostreams::bzip2_compressor() );
		else if( suffix == ".Z" )out.push( boost::iostreams::zlib_compressor() );

#ifdef HAVE_LZMA
		else if( suffix == ".xz" )out.push( boost::iostreams::lzma_compressor() );
#endif

		// write it
		out.push( output );
		boost::iostreams::copy( input, out );
	}
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Compressed();
}
