#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include <boost/foreach.hpp>
#define BOOST_FILESYSTEM_VERSION 2 //@todo switch to 3 as soon as we drop support for boost < 1.44
#include <boost/filesystem.hpp>
#include <iomanip>
#include <iostream>

#include "../CoreUtils/log.hpp"
#include "common.hpp"
#include "io_interface.h"

namespace isis
{
namespace image_io
{
namespace _internal
{
bool moreCmp( const util::istring &a, const util::istring &b ) {return a.length() > b.length();}
}

void FileFormat::write( const std::list<data::Image> &images, const std::string &filename, const std::string &dialect ) throw( std::runtime_error & )
{
	std::list<std::string> names = makeUniqueFilenames( images, filename );
	std::list<std::string>::const_iterator inames = names.begin();
	BOOST_FOREACH( std::list<data::Image>::const_reference ref, images ) {
		std::string uniquePath = *( inames++ );

		try {
			write( ref, uniquePath, dialect );
			LOG( Runtime, notice )
				<< "Image of size " << ref.getSizeAsVector() << " written to " <<  uniquePath
				<< " using " <<  getName() << (dialect.empty() ?
					std::string():
					std::string(" and dialect ")+dialect
				);
		} catch ( std::runtime_error &e ) {
			LOG( Runtime, warning )
					<< "Failed to write image to " <<  uniquePath << " using " <<  getName() << " (" << e.what() << ")";
		}
	}
}

bool FileFormat::hasOrTell( const util::PropertyMap::KeyType &name, const isis::util::PropertyMap &object, isis::LogLevel level )
{
	if ( object.hasProperty( name ) ) {
		return true;
	} else {
		LOG( Runtime, level ) << "Missing property " << util::MSubject( name );
		return false;
	}
}

void FileFormat::throwGenericError( std::string desc )
{
	throw( std::runtime_error( desc ) );
}

void FileFormat::throwSystemError( int err, std::string desc )
{
	throw( boost::system::system_error( err, boost::system::get_system_category(), desc ) );
}

std::list< util::istring > FileFormat::getSuffixes(io_modes mode)const
{
	std::list<util::istring> ret = util::stringToList<util::istring>( suffixes(mode), boost::regex( "[[:space:]]" ) );
	BOOST_FOREACH( util::istring & ref, ret ) {
		ref.erase( 0, ref.find_first_not_of( '.' ) ); // remove leading . if there are some
	}
	ret.sort( _internal::moreCmp ); //start with the longest suffix
	return ret;
}

std::pair< std::string, std::string > FileFormat::makeBasename( const std::string &filename )const
{
	std::list<util::istring> supported_suffixes = getSuffixes();
	util::istring ifilename( filename.begin(), filename.end() );
	BOOST_FOREACH( const util::istring & suffix, supported_suffixes ) {
		size_t at = ifilename.rfind( suffix );

		if( at != ifilename.npos ) {
			if( at && ifilename[at - 1] == '.' )
				at--;

			return std::make_pair( filename.substr( 0, at ), filename.substr( at ) );
		}
	}
	return std::make_pair( filename, std::string() );
}

std::string FileFormat::makeFilename( const util::PropertyMap &props, std::string namePattern )
{
	boost::regex reg( "\\{[^{}]+\\}" );
	boost::match_results<std::string::iterator> what;
	std::string::iterator pos=namePattern.begin();

	while( boost::regex_search(pos, namePattern.end() , what, reg ) ) {
		const util::PropertyMap::KeyType prop( what[0].str().substr( 1, what.length() - 2 ).c_str() );
		const std::string::iterator start=what[0].first,end=what[0].second;

		if( props.hasProperty( prop ) ) {
			const std::string pstring =  boost::regex_replace( props.getPropertyAs<std::string>( prop ), boost::regex( "[[:space:]/\\\\]" ), "_" );
			const size_t dist=start-namePattern.begin();

			namePattern.replace( start, end, pstring );
			pos=namePattern.begin()+dist+pstring.length();
			LOG( Debug, info )
					<< "Replacing " << util::PropertyMap::KeyType( "{" ) + prop + "}" << " by "   << props.getPropertyAs<std::string>( prop )
					<< " the string is now " << namePattern;
		} else {
			LOG( Runtime, warning ) << "The property " << util::MSubject( prop ) << " does not exist - ignoring it";
			namePattern.replace( start, end, "" ); // it must be removed, or it will match forever
		}
	}

	return namePattern;
}
std::list<std::string> FileFormat::makeUniqueFilenames( const std::list<data::Image> &images, const std::string &namePattern )const
{
	std::list<std::string> ret;
	std::map<std::string, unsigned short> names, used_names;
	BOOST_FOREACH( std::list<data::Image>::const_reference ref, images ) {
		names[makeFilename( ref, namePattern )]++;
	}

	BOOST_FOREACH( std::list<data::Image>::const_reference ref, images ) {
		std::string name = makeFilename( ref, namePattern );

		if( names[name] > 1 ) {
			const unsigned short number = ++used_names[name];
			const unsigned short length = ( uint16_t )log10( ( float )names[name] ) - ( uint16_t )log10( ( float )number );
			const std::string snumber = std::string( length, '0' ) + boost::lexical_cast<std::string>( number );
			const std::pair<std::string, std::string> splitted = makeBasename( name );
			name = splitted.first + "_" + snumber + splitted.second;
		}

		ret.push_back( name );
	}
	return ret;
}

const float FileFormat::invalid_float = -std::numeric_limits<float>::infinity();
}
}
