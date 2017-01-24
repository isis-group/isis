#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <boost/filesystem.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/device/file.hpp>
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

void FileFormat::write( const std::list< data::Image >& images, const std::string &filename, const util::istring &dialect, std::shared_ptr< util::ProgressFeedback > progress ) throw( std::runtime_error & )
{
	std::list<std::string> names = makeUniqueFilenames( images, filename );
	std::list<std::string>::const_iterator inames = names.begin();
	for( std::list<data::Image>::const_reference ref :  images ) {
		std::string uniquePath = *( inames++ );

		try {
			write( ref, uniquePath, dialect, progress );
			LOG( Runtime, notice )
					<< "Image of size " << util::MSubject(ref.getSizeAsVector()) << " written to " <<  util::MSubject(uniquePath)
					<< " using " <<  getName() << ( dialect.empty() ?
													util::istring() :
													util::istring( " and dialect " ) + dialect
												  );
		} catch ( std::runtime_error &e ) {
			LOG( Runtime, warning )
					<< "Failed to write image to " << util::MSubject(uniquePath) << " using " <<  getName() << " (" << e.what() << ")";
		}
	}
}

std::list<data::Chunk> FileFormat::load( const std::string &filename, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback, std::list<util::istring> format ){
	data::FilePtr ptr(filename);
	return load(ptr.getRawAddress(),ptr.getLength(),dialect,feedback,format);
}

std::list<data::Chunk> FileFormat::load(std::basic_streambuf<char> &source, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback, std::list<util::istring> format ){
	util::TmpFile tmp("isis_streamio_adapter");
	boost::iostreams::copy(source,boost::iostreams::file_sink(tmp.c_str()));
	return load(tmp.native(),dialect,feedback,format);
}

std::list<data::Chunk> FileFormat::load( std::shared_ptr<const void> source, size_t length, const util::istring &dialect, std::shared_ptr<util::ProgressFeedback> feedback, std::list<util::istring> format ){
	boost::interprocess::ibufferstream buffer(std::static_pointer_cast<const char>(source).get(),length);
	return load(*buffer.rdbuf(),dialect,feedback,format);
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
	static const std::regex reg( "\\{[^{}]+\\}", ECMAScript|optimize);
	static const std::regex regFormatInt  ( "%[-+#+]*[\\d]*[di]$", ECMAScript|optimize );
	static const std::regex regFormatUInt ( "%[-+#+]*[\\d]*[u]$",  ECMAScript|optimize );
	static const std::regex regFormatFloat( "%[-+#+]*[\\d]*(.\\d+)?[efag]$", ECMAScript|optimize| icase); 
	std::string result=namePattern;
	enum formatting_type {none,integer,uinteger,floating};
	ptrdiff_t offset=0;
	
	//NOTE: can also be done for rounding floats, but at the moment not required so not done right now


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

}
}
