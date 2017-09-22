#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "../util/log.hpp"
#include "../util/tmpfile.hpp"
#include "fileptr.hpp"
#include "common.hpp"
#include "io_interface.h"

namespace isis
{
namespace image_io
{
API_EXCLUDE_BEGIN;
/// @cond _internal
namespace _internal
{
bool moreCmp( const util::istring &a, const util::istring &b ) {return a.length() > b.length();}
}
/// @endcond _internal
API_EXCLUDE_END;

void FileFormat::write( const std::list< data::Image >& images, const std::string &filename, std::list<util::istring> dialects, std::shared_ptr< util::ProgressFeedback > progress ) throw( std::runtime_error & )
{
	std::list<std::string> names = makeUniqueFilenames( images, filename );
	std::list<std::string>::const_iterator inames = names.begin();
	for( std::list<data::Image>::const_reference ref :  images ) {
		std::string uniquePath = *( inames++ );

		try {
			write( ref, uniquePath, dialects, progress );
			LOG( Runtime, notice )
					<< "Image of size " << util::MSubject(ref.getSizeAsVector()) << " written to " <<  util::MSubject(uniquePath)
					<< " using " <<  getName();
		} catch ( std::runtime_error &e ) {
			LOG( Runtime, warning )
					<< "Failed to write image to " << util::MSubject(uniquePath) << " using " <<  getName() << " (" << e.what() << ")";
		}
	}
}

std::list<data::Chunk> FileFormat::load( const boost::filesystem::path &filename, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & ){
	//try open file
	data::FilePtr ptr(filename);
	if( !ptr.good() ) {
		if( errno ) {
			throwSystemError( errno, filename.native() + " could not be opened" );
			errno = 0;
		} else
			throwGenericError( filename.native() + " could not be opened" );
	}

	// set up progress bar if its enabled but don't fiddle with it if its set up already
	bool set_up=false;
	if( feedback && feedback->getMax() == 0 ) {
		set_up=true;
		feedback->show( boost::filesystem::file_size( filename ), std::string( "loading " ) + filename.native() );
	}
	std::list<data::Chunk> ret=load(ptr,formatstack,dialects,feedback);
	if(set_up) // close progress bar
		feedback->close();
	return ret;
}

std::list<data::Chunk> FileFormat::load(data::ByteArray source, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & ){
	typedef  boost::iostreams::basic_array_source<std::streambuf::char_type> my_source_type; // must be compatible to std::streambuf
	const void *p=source.getRawAddress().get();
	const uint8_t *start=source.begin(), *end=source.end();
	
	boost::iostreams::stream<my_source_type> stream;
	stream.open(my_source_type((const std::streambuf::char_type*)start,(const std::streambuf::char_type*)end));
	std::streambuf *buffer = stream.rdbuf();
	return load(buffer,formatstack,dialects,feedback);
}

std::list<data::Chunk> FileFormat::load(std::streambuf *source, std::list<util::istring> formatstack, std::list<util::istring> dialects, std::shared_ptr<util::ProgressFeedback> feedback )throw( std::runtime_error & ){
	util::TmpFile tmp("isis_streamio_adapter");
	boost::iostreams::copy(*source,boost::iostreams::file_sink(tmp.c_str()));
	return load(tmp.native(),formatstack,dialects,feedback);
}

bool hasOrTell( const util::PropertyMap::key_type &name, const util::PropertyMap &object, LogLevel level )
{
	if ( object.hasProperty( name ) ) {
		return true;
	} else {
		LOG( Runtime, level ) << "Missing property " << name;
		return false;
	}
}
util::PropertyMap::key_type hasOrTell(const std::initializer_list< util::PropertyMap::key_type > names, const util::PropertyMap& object, LogLevel level)
{
	for(const util::PropertyMap::key_type &key:names){ // iterate through all props
		if ( object.hasProperty( key ) )
			return key;
	}
	LOG(Runtime,level)  << "Didn't find at least one of the properties " << std::list<util::PropertyMap::key_type>(names.begin(),names.end());
	return util::PropertyMap::key_type();
}

optional< util::PropertyValue > extractOrTell(const util::PropertyMap::key_type &name, util::PropertyMap& object, LogLevel level)
{
	optional< util::PropertyValue > ret;
	boost::optional< util::PropertyValue& > found=object.queryProperty(name);
	if(found){ // if we found one, swap its contet with ret and remove it
		ret.reset(util::PropertyValue());
		found->swap(*ret);
		object.remove(name);
	}
	LOG_IF(!ret, Runtime, level ) << "Missing property " << name;
	return ret;
}
optional< util::PropertyValue > extractOrTell(const std::initializer_list< util::PropertyMap::key_type > names, util::PropertyMap& object, LogLevel level)
{
	optional< util::PropertyValue > ret;
	for(const util::PropertyMap::key_type &key:names){ // iterate through all props
		boost::optional< util::PropertyValue& > found=object.queryProperty(key);
		if(found){ // if we found one, swap its contet with ret and remove it
			ret.reset(util::PropertyValue());
			found->swap(*ret);
			object.remove(key);
			break;
		}
	}
	LOG_IF(!ret,Runtime,level)  << "Didn't find at least one of the properties " << std::list<util::PropertyMap::key_type>(names.begin(),names.end());
	return ret;
}


void FileFormat::throwGenericError( std::string desc )
{
	throw( std::runtime_error( desc ) );
}

void FileFormat::throwSystemError( int err, std::string desc )
{
	throw( boost::system::system_error( err, boost::system::get_system_category(), desc ) );
}

std::list< util::istring > FileFormat::getSuffixes( io_modes mode )const
{
	std::list<util::istring> ret = util::stringToList<util::istring>( suffixes( mode ).c_str() );
	for( util::istring & ref :  ret ) {
		ref.erase( 0, ref.find_first_not_of( '.' ) ); // remove leading . if there are some
	}
	ret.sort( _internal::moreCmp ); //start with the longest suffix
	return ret;
}

std::pair< std::string, std::string > FileFormat::makeBasename( const std::string &filename )const
{
	std::list<util::istring> supported_suffixes = getSuffixes();
	util::istring ifilename( filename.begin(), filename.end() );
	for( const util::istring & suffix :  supported_suffixes ) {
		util::istring check = ifilename.substr( ifilename.length() - suffix.length(), suffix.length() );

		if( filename[filename.length() - suffix.length() - 1] == '.' && check == suffix ) {
			return std::make_pair( filename.substr( 0, filename.length() - suffix.length() - 1 ), filename.substr( filename.length() - suffix.length() - 1 ) );
		}
	}
	return std::make_pair( filename, std::string() );
}

// @todo document me (esp. the formatting-stuff)
std::string FileFormat::makeFilename( const util::PropertyMap &props, const std::string namePattern )
{
	using namespace std::regex_constants;
	static const std::regex reg( "\\{[^{}]+\\}", optimize);
	static const std::regex invalid( "[/\\\\[:space:]]", optimize);
	static const std::regex regFormatInt  ( "%[-+#+]*[\\d]*[di]$", optimize );
	static const std::regex regFormatUInt ( "%[-+#+]*[\\d]*[u]$",  optimize );
	static const std::regex regFormatFloat( "%[-+#+]*[\\d]*(.\\d+)?[efag]$", optimize| icase); 
	std::string result=namePattern;
	enum formatting_type {none,integer,uinteger,floating};
	ptrdiff_t offset=0;
	
	//iterate through all {whatever} in the pattern
	for(std::sregex_iterator i=std::sregex_iterator(namePattern.begin(),namePattern.end(), reg); i!=std::sregex_iterator();i++){
		std::string smatch=i->str().substr(1,i->length()-2); // get the string inside the {}
		std::smatch fwhat;
		std::string format;
		
		//check if we have a printf formatting in there
		formatting_type formatting =
			std::regex_search( smatch, fwhat, regFormatInt ) ? integer:
			std::regex_search( smatch, fwhat, regFormatUInt ) ? uinteger:
			std::regex_search( smatch, fwhat, regFormatFloat ) ? floating:
			none;
		
		if(formatting!=none){ 
			format=fwhat[0].str();
			smatch.erase(fwhat[0].first,fwhat[0].second); // remove it
		}
		
		util::PropertyMap::key_type prop( smatch.c_str() ); // use remaining string to look for property
		const boost::optional< util::PropertyValue const& > found= props.queryProperty( prop );
		std::string pstring;
		if( found && !found->isEmpty() ) {
			if(formatting==none){
				pstring=found->toString();
			} else {
				char buffer[1024];
				switch(formatting){
				case integer:
					std::snprintf(buffer,1024,format.c_str(),found->as<int64_t>());
					break;
				case uinteger:
					std::snprintf(buffer,1024,format.c_str(),found->as<uint64_t>());
					break;
				case  floating:
					std::snprintf(buffer,1024,format.c_str(),found->as<double>());
					break;
				case none:;//should never happen / just to stop compiler from whining about missing 'none'
				} 
				pstring=std::string(buffer);
			}
			pstring=std::regex_replace(pstring,invalid,"_");
			result.replace(i->position()+offset, i->length(),pstring);
			offset+=pstring.length()-i->length();
		} else
			LOG(Runtime,warning) << "Won't replace " << i->str() << " as there is no such property";
	}

	return result;
}

std::list<std::string> FileFormat::makeUniqueFilenames( const std::list<data::Image> &images, const std::string &namePattern )const
{
	std::list<std::string> ret;
	std::map<std::string, unsigned short> names, used_names;
	for( std::list<data::Image>::const_reference ref :  images ) {
		names[makeFilename( ref, namePattern )]++;
	}

	for( std::list<data::Image>::const_reference ref :  images ) {
		std::string name = makeFilename( ref, namePattern );

		if( names[name] > 1 ) {
			const unsigned short number = ++used_names[name];
			const unsigned short length = ( uint16_t )log10( ( float )names[name] ) - ( uint16_t )log10( ( float )number );
			const std::string snumber = std::string( length, '0' ) + std::to_string( number );
			const std::pair<std::string, std::string> splitted = makeBasename( name );
			name = splitted.first + "_" + snumber + splitted.second;
		}

		ret.push_back( name );
	}
	return ret;
}

bool FileFormat::checkDialect(const std::list<util::istring> &dialects,util::istring searched){
	return std::find(dialects.begin(),dialects.end(),searched)!=dialects.end();
}

}
}
