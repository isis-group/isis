
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
namespace _internal{

class progress_filter {
public:
	progress_filter(util::ProgressFeedback &feedback):m_feedback(feedback),remain(0){}
	util::ProgressFeedback &m_feedback;
	std::streamsize remain;
	static const std::streamsize blocksize=0x10000;//64k
	typedef char char_type;
	
	struct category : boost::iostreams::dual_use_filter_tag, boost::iostreams::multichar_tag{ };

	void progress(std::streamsize n){
		remain+=n;
		if(remain>=blocksize){
			m_feedback.progress("",remain/blocksize);
			remain=remain%blocksize;
		}
	}
	template<typename Source>
	std::streamsize read(Source& src, char* s, std::streamsize n)
	{
		progress(n);
		return boost::iostreams::read(src,s,n);
	}
	template<typename Sink>
	std::streamsize write(Sink& dest, const char* s, std::streamsize n)
	{
		progress(n);;
		return boost::iostreams::write(dest,s,n);
	}
};
}

class ImageFormat_Compressed: public FileFormat
{
protected:
	util::istring suffixes( io_modes /*modes = both*/ )const {return "gz bz2 Z";}
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
		std::list<data::Chunk>::iterator prev = chunks.end();--prev; //memory current position in the output list
		std::pair< std::string, std::string > proxyBase = makeBasename( filename );
		const util::istring suffix = proxyBase.second.c_str();

		const data::IOFactory::FileFormatList formats = data::IOFactory::getFileFormatList( proxyBase.first, dialect );
		
		if( formats.empty() ) {
			throwGenericError( "Cannot determine the uncompressed suffix of \"" + filename + "\" because no io-plugin was found for it" );
		}
		
		// set up the input stream
		util::TmpFile tmpFile( "", formats.front()->makeBasename( proxyBase.first ).second );
		std::ofstream output(tmpFile.file_string().c_str(), std::ios_base::binary );
		std::ifstream input( filename.c_str(), std::ios_base::binary );
		input.exceptions( std::ios::badbit );
		output.exceptions( std::ios::badbit );
		
		boost::iostreams::filtering_istream in;

		if( suffix == ".gz" )in.push( boost::iostreams::gzip_decompressor() );
		else if( suffix == ".bz2" )in.push( boost::iostreams::bzip2_decompressor() );
		else if( suffix == ".Z" )in.push( boost::iostreams::zlib_decompressor() );

		if(progress){
			progress->show(boost::filesystem::file_size(filename)/_internal::progress_filter::blocksize,std::string("decompressing ")+filename);
			in.push( _internal::progress_filter(*progress));
		}
		in.push( input );
		boost::iostreams::copy(in,output);

		int ret = data::IOFactory::load( chunks, tmpFile.file_string().c_str(), dialect );
		
		if( ret ) { //re-set source of all new chunks
			prev++;
			LOG( Debug, info ) <<  "Setting source of all " << std::distance( prev, chunks.end() ) << " chunks to " << util::MSubject( filename );
			
			for( ; prev != chunks.end(); ++prev ) {
				prev->setPropertyAs( "source", filename );
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
		if(!data::IOFactory::write(image,tmpFile.file_string(),dialect)){throwGenericError(tmpFile.file_string()+" failed to write");} 

		// set up the compression stream
		std::ifstream input(tmpFile.file_string().c_str(), std::ios_base::binary );
		std::ofstream output( filename.c_str(), std::ios_base::binary );
		input.exceptions( std::ios::badbit );
		output.exceptions( std::ios::badbit );
		
		boost::iostreams::filtering_ostream out;

		if(progress){
			progress->show(boost::filesystem::file_size(tmpFile)/_internal::progress_filter::blocksize,std::string("compressing ")+filename);
			out.push( _internal::progress_filter(*progress));
		}
		
		if( suffix == ".gz" )out.push( boost::iostreams::gzip_compressor() );
		else if( suffix == ".bz2" )out.push( boost::iostreams::bzip2_compressor() );
		else if( suffix == ".Z" )out.push( boost::iostreams::zlib_compressor() );

		// write it
		out.push( output );
		boost::iostreams::copy(input,out);
	}
	bool tainted()const {return false;}//internal plugins are not tainted
};
}
}
isis::image_io::FileFormat *factory()
{
	return new isis::image_io::ImageFormat_Compressed();
}
