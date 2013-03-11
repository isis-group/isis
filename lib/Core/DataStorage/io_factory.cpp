//
// C++ Implementation: io_factory
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "io_factory.hpp"
#ifdef WIN32
#include <windows.h>
#include <Winbase.h>
#define BOOST_FILESYSTEM_VERSION 2 //@todo switch to 3 as soon as we drop support for boost < 1.44
#include <boost/filesystem/path.hpp>
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include <vector>
#include <algorithm>

#include "../CoreUtils/log.hpp"
#include "common.hpp"
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/system/error_code.hpp>
#include <boost/algorithm/string.hpp>
#include "../CoreUtils/singletons.hpp"

namespace isis
{
namespace data
{
API_EXCLUDE_BEGIN
/// @cond _internal
namespace _internal
{
struct pluginDeleter {
	void *m_dlHandle;
	std::string m_pluginName;
	pluginDeleter( void *dlHandle, std::string pluginName ): m_dlHandle( dlHandle ), m_pluginName( pluginName ) {}
	void operator()( image_io::FileFormat *format ) {
		delete format;
#ifdef WIN32

		if( !FreeLibrary( ( HINSTANCE )m_dlHandle ) )
#else
		if ( dlclose( m_dlHandle ) != 0 )
#endif
			std::cerr << "Failed to release plugin " << m_pluginName << " (was loaded at " << m_dlHandle << ")";

		//we cannot use LOG here, because the loggers are gone allready
	}
};
struct dialect_missing {
	util::istring dialect;
	std::string filename;
	bool operator()( IOFactory::FileFormatList::reference ref )const {
		const util::istring dia = ref->dialects( filename );
		std::list<util::istring> splitted = util::stringToList<util::istring>( dia, ' ' );
		const bool ret = ( std::find( splitted.begin(), splitted.end(), dialect ) == splitted.end() );
		LOG_IF( ret, image_io::Runtime, warning ) << ref->getName() << " does not support the requested dialect " << util::MSubject( dialect );
		return ret;
	}
};

bool invalid_and_tell( Chunk &candidate )
{
	LOG_IF( !candidate.isValid(), image_io::Runtime, error ) << "Ignoring invalid chunk. Missing properties: " << candidate.getMissing();
	return !candidate.isValid();
}

}
/// @endcond _internal
API_EXCLUDE_BEGIN

IOFactory::IOFactory()
{
	const char *env_path = getenv( "ISIS_PLUGIN_PATH" );
	const char *env_home = getenv( "HOME" );

	if( env_path ) {
		findPlugins( boost::filesystem::path( env_path ).directory_string() );
	}

	if( env_home ) {
		const boost::filesystem::path home = boost::filesystem::path( env_home ) / "isis" / "plugins";

		if( boost::filesystem::exists( home ) ) {
			findPlugins( home.directory_string() );
		} else {
			LOG( Runtime, info ) << home.directory_string() << " does not exist. Won't check for plugins there";
		}
	}

#ifdef WIN32
	TCHAR lpExeName[2048];
	DWORD lExeName = GetModuleFileName( NULL, lpExeName, 2048 );
	bool w32_path_ok = false;

	if( lExeName == 0 ) {
		LOG( Runtime, error ) << "Failed to get the process name " << util::MSubject( util::getLastSystemError() );
	} else if( lExeName < 2048 ) {
		lpExeName[lExeName] = '\0';
		boost::filesystem::path prog_name( lpExeName );

		if( boost::filesystem::exists( prog_name ) ) {
			w32_path_ok = true;
			LOG( Runtime, info ) << "Determined the path of the executable as " << util::MSubject( prog_name.remove_filename().directory_string() ) << " will search for plugins there..";
			findPlugins( prog_name.remove_filename().directory_string() );
		}
	} else
		LOG( Runtime, error ) << "Sorry, the path of the process is to long (must be less than 2048 characters) ";

	LOG_IF( !w32_path_ok, Runtime, warning ) << "Could not determine the path of the executable, won't search for plugins there..";
#else
	findPlugins( std::string( PLUGIN_PATH ) );
#endif
}

bool IOFactory::registerFileFormat( const FileFormatPtr plugin )
{
	if ( !plugin )return false;

	io_formats.push_back( plugin );
	std::list<util::istring> suffixes = plugin->getSuffixes(  );
	LOG( Runtime, info )
			<< "Registering " << ( plugin->tainted() ? "tainted " : "" ) << "io-plugin "
			<< util::MSubject( plugin->getName() )
			<< " with supported suffixes " << suffixes;
	BOOST_FOREACH( util::istring & it, suffixes ) {
		io_suffix[it].push_back( plugin );
	}
	return true;
}

unsigned int IOFactory::findPlugins( const std::string &path )
{
	boost::filesystem::path p( path );

	if ( !exists( p ) ) {
		LOG( Runtime, warning ) << util::MSubject( p.file_string() ) << " not found";
		return 0;
	}

	if ( !boost::filesystem::is_directory( p ) ) {
		LOG( Runtime, warning ) << util::MSubject( p.file_string() ) << " is no directory";
		return 0;
	}

	LOG( Runtime, info )   << "Scanning " << util::MSubject( p ) << " for plugins";
	boost::regex pluginFilter( std::string( "^" ) + DL_PREFIX + "isisImageFormat_" + "[[:word:]]+" + DL_SUFFIX + "$" );
	unsigned int ret = 0;

	for ( boost::filesystem::directory_iterator itr( p ); itr != boost::filesystem::directory_iterator(); ++itr ) {
		if ( boost::filesystem::is_directory( *itr ) )continue;

		if ( boost::regex_match( itr->path().leaf(), pluginFilter ) ) {
			const std::string pluginName = itr->path().file_string();
#ifdef WIN32
			HINSTANCE handle = LoadLibrary( pluginName.c_str() );
#else
			void *handle = dlopen( pluginName.c_str(), RTLD_NOW );
#endif

			if ( handle ) {
#ifdef WIN32
				image_io::FileFormat* ( *factory_func )() = ( image_io::FileFormat * ( * )() )GetProcAddress( handle, "factory" );
#else
				image_io::FileFormat* ( *factory_func )() = ( image_io::FileFormat * ( * )() )dlsym( handle, "factory" );
#endif

				if ( factory_func ) {
					FileFormatPtr io_class( factory_func(), _internal::pluginDeleter( handle, pluginName ) );

					if ( registerFileFormat( io_class ) ) {
						io_class->plugin_file = pluginName;
						ret++;
					} else {
						LOG( Runtime, warning ) << "failed to register plugin " << util::MSubject( pluginName );
					}
				} else {
#ifdef WIN32
					LOG( Runtime, warning )
							<< "could not get format factory function from " << util::MSubject( pluginName );
					FreeLibrary( handle );
#else
					LOG( Runtime, warning )
							<< "could not get format factory function from " << util::MSubject( pluginName ) << ":" << util::MSubject( dlerror() );
					dlclose( handle );
#endif
				}
			} else
#ifdef WIN32
				LOG( Runtime, warning ) << "Could not load library " << util::MSubject( pluginName );

#else
				LOG( Runtime, warning ) << "Could not load library " << util::MSubject( pluginName ) << ":" <<  util::MSubject( dlerror() );
#endif
		} else {
			LOG( Runtime, verbose_info ) << "Ignoring " << *itr << " because it doesn't match " << pluginFilter.str();
		}
	}

	return ret;
}

IOFactory &IOFactory::get()
{
	return util::Singletons::get<IOFactory, INT_MAX>();
}

size_t IOFactory::loadFile( std::list<Chunk> &ret, const boost::filesystem::path &filename, util::istring suffix_override, util::istring dialect )
{
	FileFormatList formatReader;
	formatReader = getFileFormatList( filename.file_string(), suffix_override, dialect );
	const size_t nimgs_old = ret.size();   // save number of chunks
	const util::istring with_dialect = dialect.empty() ?
									   util::istring( "" ) : util::istring( " with dialect \"" ) + dialect + "\"";

	if ( formatReader.empty() ) {
		if( !boost::filesystem::exists( filename ) ) {
			LOG( Runtime, error ) << util::MSubject( filename.file_string() )
								  << " does not exist as file, and no suitable plugin was found to generate data from "
								  << ( suffix_override.empty() ? util::istring( "that name" ) : util::istring( "the suffix \"" ) + suffix_override + "\"" );
		} else if( suffix_override.empty() ) {
			LOG( Runtime, error ) << "No plugin found to read " << filename.file_string() << with_dialect;
		} else {
			LOG( Runtime, error ) << "No plugin supporting the requested suffix " << suffix_override << with_dialect << " was found";
		}
	} else {
		BOOST_FOREACH( FileFormatList::const_reference it, formatReader ) {
			LOG( ImageIoDebug, info )
					<< "plugin to load file" << with_dialect << " " << util::MSubject( filename.file_string() ) << ": " << it->getName();

			try {
				return it->load( ret, filename.file_string(), dialect, m_feedback );
			} catch ( std::runtime_error &e ) {
				if( suffix_override.empty() ) {
					LOG( Runtime, formatReader.size() > 1 ? warning : error )
							<< "Failed to load " <<  filename.file_string() << " using " <<  it->getName() << with_dialect << " ( " << e.what() << " )";
				} else {
					LOG( Runtime, warning )
							<< "The enforced format " << it->getName()  << " failed to read " << filename.file_string() << with_dialect
							<< " ( " << e.what() << " ), maybe it just wasn't the right format";
				}
			}
		}
		LOG_IF( boost::filesystem::exists( filename ) && formatReader.size() > 1, Runtime, error ) << "No plugin was able to load: "   << util::MSubject( filename.file_string() ) << with_dialect;
	}

	return ret.size() - nimgs_old;//no plugin of proposed list could load file
}


IOFactory::FileFormatList IOFactory::getFileFormatList( std::string filename, util::istring suffix_override, util::istring dialect )
{
	std::list<std::string> ext;
	FileFormatList ret;
	_internal::dialect_missing remove_op;

	if( suffix_override.empty() ) { // detect suffixes from the filename
		const boost::filesystem::path fname( filename );
		ext = util::stringToList<std::string>( fname.leaf(), '.' ); // get all suffixes

		if( !ext.empty() )ext.pop_front(); // remove the first "suffix" - actually the basename
	} else ext = util::stringToList<std::string>( suffix_override, '.' );

	while( !ext.empty() ) {
		const util::istring wholeName( util::listToString( ext.begin(), ext.end(), ".", "", "" ).c_str() ); // (re)construct the rest of the suffix
		const std::map<util::istring, FileFormatList>::iterator found = get().io_suffix.find( wholeName );

		if( found != get().io_suffix.end() ) {
			LOG( Debug, verbose_info ) << found->second.size() << " plugins support suffix " << wholeName;
			ret.insert( ret.end(), found->second.begin(), found->second.end() );
		}

		ext.pop_front();
	}

	if( dialect.empty() ) {
		LOG_IF( ret.size() > 1, Debug, info ) << "No dialect given. Will use all " << ret.size() << " plugins";
	} else {//remove everything which lacks the dialect if there was some given
		remove_op.dialect = dialect;
		remove_op.filename = filename;
		ret.remove_if( remove_op );
		LOG( Debug, info ) << "Removed everything which does not support the dialect " << util::MSubject( dialect ) << " on " << filename << "(" << ret.size() << " plugins left)";
	}

	return ret;
}

std::list< Image > IOFactory::chunkListToImageList( std::list<Chunk> &src )
{
	// throw away invalid chunks
	size_t errcnt = src.size();
	src.remove_if( _internal::invalid_and_tell );
	errcnt -= src.size();

	std::list< Image > ret;

	while ( !src.empty() ) {
		LOG( Debug, info ) << src.size() << " Chunks left to be distributed.";
		size_t before = src.size();

		Image buff( src );

		if ( buff.isClean() ) {
			if( buff.isValid() ) { //if the image was successfully indexed and is valid, keep it
				ret.push_back( buff );
				LOG( Runtime, info ) << "Image " << ret.size() << " with size " << util::MSubject( buff.getSizeAsString() ) << " done.";
			} else {
				LOG_IF( !buff.getMissing().empty(), Runtime, error )
						<< "Cannot insert image. Missing properties: " << buff.getMissing();
				errcnt += before - src.size();
			}
		} else
			LOG( Runtime, info ) << "Dropping non clean Image";
	}

	LOG_IF( errcnt, Runtime, warning ) << "Dropped " << errcnt << " chunks because they didn't form valid images";
	return ret;
}

size_t IOFactory::load( std::list<data::Chunk> &chunks, const std::string &path, util::istring suffix_override, util::istring dialect )
{
	const boost::filesystem::path p( path );
	const size_t loaded = boost::filesystem::is_directory( p ) ?
						  get().loadPath( chunks, p, suffix_override, dialect ) :
						  get().loadFile( chunks, p, suffix_override, dialect );
	BOOST_FOREACH( Chunk & ref, chunks ) {
		if ( ! ref.hasProperty( "source" ) )
			ref.setPropertyAs( "source", p.file_string() );
	}
	return loaded;
}

std::list< Image > IOFactory::load ( const util::slist &paths, util::istring suffix_override, util::istring dialect )
{
	std::list<Chunk> chunks;
	size_t loaded = 0;
	BOOST_FOREACH( const std::string & path, paths ) {
		loaded += load( chunks, path , suffix_override, dialect );
	}
	const std::list<data::Image> images = chunkListToImageList( chunks );
	LOG( Runtime, info )
			<< "Generated " << images.size() << " images out of " << loaded << " chunks loaded from " << paths;
	return images;
}

std::list<data::Image> IOFactory::load( const std::string &path, util::istring suffix_override, util::istring dialect )
{
	return load( util::slist( 1, path ), suffix_override, dialect );
}

size_t IOFactory::loadPath( std::list<Chunk> &ret, const boost::filesystem::path &path, util::istring suffix_override, util::istring dialect )
{
	int loaded = 0;

	if( m_feedback ) {
		const size_t length = std::distance( boost::filesystem::directory_iterator( path ), boost::filesystem::directory_iterator() ); //@todo this will also count directories
		m_feedback->show( length, std::string( "Reading " ) + util::Value<std::string>( length ).toString( false ) + " files from " + path.file_string() );
	}

	for ( boost::filesystem::directory_iterator i( path ); i != boost::filesystem::directory_iterator(); ++i )  {
		if ( boost::filesystem::is_directory( *i ) )continue;

		loaded += loadFile( ret, *i, suffix_override, dialect );

		if( m_feedback )
			m_feedback->progress();
	}

	if( m_feedback )
		m_feedback->close();

	return loaded;
}

bool IOFactory::write( const data::Image &image, const std::string &path, util::istring suffix_override, util::istring dialect )
{
	return write( std::list<data::Image>( 1, image ), path, suffix_override, dialect );
}


bool IOFactory::write( std::list< isis::data::Image > images, const std::string &path, util::istring suffix_override, util::istring dialect )
{
	const FileFormatList formatWriter = get().getFileFormatList( path, suffix_override, dialect );

	BOOST_FOREACH( std::list<data::Image>::reference ref, images ) {
		ref.checkMakeClean();
	}

	if( formatWriter.size() ) {
		BOOST_FOREACH( FileFormatList::const_reference it, formatWriter ) {
			LOG( Debug, info )
					<< "plugin to write to " <<  path << ": " << it->getName()
					<<  ( dialect.empty() ?
						  util::istring( "" ) :
						  util::istring( " using dialect: " ) + dialect
						);

			try {
				it->write( images, path, dialect, get().m_feedback );
				LOG( Runtime, info )
						<< images.size()
						<< " images written to " << path << " using " <<  it->getName()
						<<  ( dialect.empty() ?
							  util::istring( "" ) :
							  util::istring( " and dialect: " ) + dialect
							);
				return true;
			} catch ( std::runtime_error &e ) {
				LOG( Runtime, warning )
						<< "Failed to write " <<  images.size()
						<< " images to " << path << " using " <<  it->getName() << " (" << e.what() << ")";
			}
		}
	} else {
		LOG( Runtime, error ) << "No plugin found to write to: " << path; //@todo error message missing
	}

	return false;
}
void IOFactory::setProgressFeedback( boost::shared_ptr<util::ProgressFeedback> feedback )
{
	IOFactory &This = get();

	if( This.m_feedback )This.m_feedback->close();

	This.m_feedback = feedback;
}

IOFactory::FileFormatList IOFactory::getFormats()
{
	return get().io_formats;
}


}
} // namespaces data isis
