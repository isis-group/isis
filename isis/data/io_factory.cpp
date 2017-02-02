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
#else
	#include <dlfcn.h>
#endif
#include <boost/filesystem.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

#include "../util/log.hpp"
#include "common.hpp"
#include "../util/singletons.hpp"


namespace isis
{
namespace data
{

IOFactory::io_error::io_error(const char *what,FileFormatPtr format):std::runtime_error(what),p_format(format){}
IOFactory::FileFormatPtr IOFactory::io_error::which()const{
	return p_format;
}

IOFactory::IOFactory()
{
	const char *env_path = getenv( "ISIS_PLUGIN_PATH" );
	const char *env_home = getenv( "HOME" );

	if( env_path ) {
		findPlugins( boost::filesystem::path( env_path ).native() );
	}

	if( env_home ) {
		const boost::filesystem::path home = boost::filesystem::path( env_home ) / "isis" / "plugins";

		if( boost::filesystem::exists( home ) ) {
			findPlugins( home.native() );
		} else {
			LOG( Runtime, info ) << home << " does not exist. Won't check for plugins there";
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
			<< "Registering " << util::NoSubject( plugin->tainted() ? "tainted " : "" ) << "io-plugin "
			<< util::MSubject( plugin->getName() )
			<< " with supported suffixes " << suffixes;
	for( util::istring & it :  suffixes ) {
		io_suffix[it].push_back( plugin );
	}
	return true;
}

unsigned int IOFactory::findPlugins( const std::string &path )
{
	boost::filesystem::path p( path );

	if ( !exists( p ) ) {
		LOG( Runtime, warning ) << util::MSubject( p ) << " not found";
		return 0;
	}

	if ( !boost::filesystem::is_directory( p ) ) {
		LOG( Runtime, warning ) << util::MSubject( p ) << " is no directory";
		return 0;
	}

	LOG( Runtime, info )   << "Scanning " << util::MSubject( p ) << " for plugins";
	static const std::string pluginFilterStr=std::string(DL_PREFIX)+"isisImageFormat_[\\w]+"+DL_SUFFIX;
	static const std::regex pluginFilter( pluginFilterStr, std::regex_constants::ECMAScript | std::regex_constants::icase);
	unsigned int ret = 0;

	for ( boost::filesystem::directory_iterator itr( p ); itr != boost::filesystem::directory_iterator(); ++itr ) {
		if ( boost::filesystem::is_directory( *itr ) )continue;

		if ( std::regex_match( itr->path().filename().string(), pluginFilter ) ) {
			const std::string pluginName = itr->path().native();
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

				auto deleter = [handle, pluginName]( image_io::FileFormat *format ) {
					delete format;
#ifdef WIN32
					if( !FreeLibrary( ( HINSTANCE )handle ) )
						std::cerr << "Failed to release plugin " << pluginName << " (was loaded at " << handle << ")";
					// TODO we cannot use LOG here, because the loggers are gone allready
#else
					if ( dlclose( handle ) != 0 )
						std::cerr << "Failed to release plugin " << pluginName << " (was loaded at " << handle << ")";
					// TODO we cannot use LOG here, because the loggers are gone allready
#endif
				};

				if ( factory_func ) {
					FileFormatPtr io_class( factory_func(), deleter );

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
			LOG( Runtime, verbose_info ) << "Ignoring " << *itr << " because it doesn't match " << pluginFilterStr;
		}
	}

	return ret;
}

IOFactory &IOFactory::get()
{
	return util::Singletons::get<IOFactory, INT_MAX>();
}

std::list<Chunk> IOFactory::load_impl(const load_source &v, std::list<util::istring> formatstack, util::istring dialect)throw( io_error & ){
	bool overridden=true;
	const boost::filesystem::path* filename = boost::get<boost::filesystem::path>( &v );
	if(formatstack.empty()){
		if(filename){
			overridden=false;
			formatstack=getFormatStack(filename->string());
		}
	}
	FileFormatList readerList = getFileFormatList(formatstack , dialect );
	const util::NoSubject with_dialect = dialect.empty() ?
	                                   util::NoSubject( "" ) : util::NoSubject(util::istring( " with dialect \"" ) + dialect + "\"");
	if ( readerList.empty() ) {
		if(filename && !boost::filesystem::exists( *filename ) ) { // if it actually is a filename, but does not exist
			LOG( Runtime, error ) 
				<< util::MSubject( *filename )
				<< " does not exist as file, and no suitable plugin was found to generate data from "
				<< ( overridden ? 
						util::istring( "the requested format stack \"" ) + util::listToString<util::istring>(formatstack.begin(),formatstack.end()) + "\"": 
						util::istring( "that name" )
				);

		} else if( overridden ) {
			LOG( Runtime, error ) << "No plugin supporting the requested format stack " << formatstack << with_dialect << " was found";
		} else {
			LOG_IF(filename, Runtime, error ) << "No plugin found to read " << *filename << with_dialect;
			LOG_IF(boost::get<std::basic_streambuf<char>*>(&v), Runtime, error ) << "No plugin found to load from stream " << with_dialect;
			LOG_IF(boost::get<ValueArray<uint8_t>>(&v), Runtime, error ) << "No plugin found to load from memory " << with_dialect;
		}
	} else {
		while( !readerList.empty() ) {
			FileFormatPtr format=readerList.front();
			readerList.pop_front();
			LOG_IF(filename, ImageIoDebug, info ) << "plugin to load file" << with_dialect << " " << filename << ": " << format->getName();

			try {
				std::list<data::Chunk> loaded =boost::apply_visitor(
					[&](auto val) { return format->load( val, formatstack, dialect, m_feedback ); },
					v
				);
				if(filename){
					for( Chunk & ref :  loaded ) {
						ref.refValueAsOr( "source", filename->native() ); // set source to filename or leave it if its already set
					}
				}
				return loaded;
			} catch ( const std::runtime_error& e ) {
				if(readerList.empty()) // if it was the last reader drop through the error
					std::throw_with_nested(IOFactory::io_error(e.what(),format));
				else if( overridden ) {
					LOG_IF(filename, Runtime, notice )
						<< "The enforced format " << format->getName()  << " failed to read " << *filename << with_dialect
						<< " with " << e.what() << ", maybe it just wasn't the right format";
				} else {
					LOG( Runtime, notice )
						<< "Failed to load " <<  *filename << " using " <<  format->getName() << with_dialect << " ( " << e.what() << " )";
				}
			}
		}
	}
}

std::list<util::istring> IOFactory::getFormatStack( std::string filename ){
	const boost::filesystem::path fname( filename );
	std::list<util::istring> ret = util::stringToList<util::istring>( fname.filename().string(), '.' ); // get all suffixes (and the basename)
	if( !ret.empty() )ret.pop_front(); // remove the basename
	return ret;
}

IOFactory::FileFormatList IOFactory::getFileFormatList( std::list<util::istring> format, util::istring dialect )
{
	FileFormatList ret;
	std::list<util::istring> buffer=format;

	while( !buffer.empty() ) {
		const util::istring wholeName=util::listToString<util::istring>( buffer.begin(), buffer.end(), ".", "", "" ); // (re)construct the rest of the suffix
		const std::map<util::istring, FileFormatList>::iterator found = get().io_suffix.find( wholeName );

		if( found != get().io_suffix.end() ) {
			LOG( Debug, verbose_info ) << found->second.size() << " plugins support suffix " << wholeName;
			ret.insert( ret.end(), found->second.begin(), found->second.end() );
		}
		buffer.pop_front(); // remove one suffix, and try again
	}

	if( dialect.empty() ) {
		LOG_IF( ret.size() > 1, Debug, info ) << "No dialect given. Will use all " << ret.size() << " plugins";
	} else {//remove everything which lacks the dialect if there was some given
		auto remove_op = [dialect, format](FileFormatList::reference ref){
			const util::istring dia = ref->dialects( format );
			std::list<util::istring> splitted = util::stringToList<util::istring>( dia, ' ' );
			const bool ret = ( std::find( splitted.begin(), splitted.end(), dialect ) == splitted.end() );
			LOG_IF( ret, image_io::Runtime, warning ) << ref->getName() << " does not support the requested dialect " << util::MSubject( dialect );
			return ret;
		};
		ret.remove_if( remove_op );
		LOG( Debug, info ) << "Removed everything which does not support the dialect " << util::MSubject( dialect ) << " on the format " << format << "(" << ret.size() << " plugins left)";
	}
	return ret;
}

std::list< Image > IOFactory::chunkListToImageList( std::list<Chunk> &src, optional< util::slist& > rejected )
{
	LOG_IF(src.empty(),Debug,warning) << "Calling chunkListToImageList with an empty chunklist";
	// throw away invalid chunks
	size_t errcnt=0;
	for(std::list<Chunk>::iterator i=src.begin();i!=src.end();){
		if(!i->isValid()){
			LOG(image_io::Runtime, error ) << "Rejecting invalid chunk. Missing properties: " << i->getMissing();
			errcnt++;
			if(rejected)
				rejected->push_back(i->getValueAs<std::string>("source")); 
			src.erase(i++);//we must increment i before the removal, otherwise it will be invalid
		} else
			i++;
	}

	std::list< Image > ret;

	while ( !src.empty() ) {
		LOG( Debug, info ) << src.size() << " Chunks left to be distributed.";
		size_t before = src.size();

		Image buff( src, rejected );

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

std::list< Chunk > IOFactory::loadChunks( const load_source &v, std::list<util::istring> formatstack, isis::util::istring dialect)throw( io_error & )
{
	const boost::filesystem::path* filename = boost::get<boost::filesystem::path>( &v );
	if(filename)
		assert(!boost::filesystem::is_directory( *filename ));
	return get().load_impl( v, formatstack, dialect );
}

std::list< Image > IOFactory::load( const util::slist &paths, std::list<util::istring> formatstack, util::istring dialect, optional< isis::util::slist& > rejected )
{
	std::list<Chunk> chunks;
	for( const std::string & path :  paths ) {
		std::list<Chunk> loaded;
		if(boost::filesystem::is_directory( path )){
			loaded=get().loadPath(path,formatstack,dialect,rejected);
		} else {
			try {
				loaded=get().load_impl( path , formatstack, dialect);
				if(loaded.empty() && rejected)
					rejected->push_back(path);
			} catch(const io_error &e) {
				const util::NoSubject with_dialect = dialect.empty() ?
					util::NoSubject( "" ) : util::NoSubject(util::istring( " with dialect \"" ) + dialect + "\"");

				LOG( Runtime, notice )
					<< "Failed to load " <<  path << " using " <<  e.which() << with_dialect << " ( " << e.what() << " )";
			}
		}
		chunks.splice(chunks.end(),loaded);
	}
	const std::list<data::Image> images = chunkListToImageList( chunks, rejected );
	LOG( Runtime, info ) << "Generated " << images.size() << " images out of " << paths;

	// store paths of red, but rejected chunks
	std::set<std::string> image_rej;
	for(const data::Chunk &ch :  chunks ){
		image_rej.insert(ch.getValueAs<std::string>("source"));
	}
	if(rejected)
		rejected->insert(rejected->end(),image_rej.begin(),image_rej.end());
	
	return images;
}

std::list<data::Image> IOFactory::load( const std::string &path, std::list<util::istring> formatstack, util::istring dialect, optional< isis::util::slist& > rejected )
{
	return load( {path}, formatstack, dialect );
}

std::list<Chunk> isis::data::IOFactory::loadPath(const boost::filesystem::path& path, std::list<util::istring> formatstack, util::istring dialect, optional< isis::util::slist& > rejected )
{
	std::list<Chunk> ret;

	if( m_feedback ) {
		const size_t length = std::distance( boost::filesystem::directory_iterator( path ), boost::filesystem::directory_iterator() ); //@todo this will also count directories
		m_feedback->show( length, std::string( "Reading " ) + util::Value<std::string>( length ).toString( false ) + " files from " + path.native() );
	}

	for ( boost::filesystem::directory_iterator i( path ); i != boost::filesystem::directory_iterator(); ++i )  {
		if ( boost::filesystem::is_directory( *i ) )continue;

		try {
			std::list<Chunk> loaded= load_impl( *i, formatstack, dialect );

			if(rejected && loaded.empty()){
				rejected->push_back(boost::filesystem::path(*i).native());
			}
			ret.splice(ret.end(),loaded);
		} catch(const io_error &e) {
			const util::NoSubject with_dialect = dialect.empty() ?
				util::NoSubject( "" ) : util::NoSubject(util::istring( " with dialect \"" ) + dialect + "\"");

			LOG( Runtime, notice )
				<< "Failed to load " <<  *i << " using " <<  e.which() << with_dialect << " ( " << e.what() << " )";
		}

		if( m_feedback )
			m_feedback->progress();
		
	}

	if( m_feedback )
		m_feedback->close();

	return ret;
}

bool IOFactory::write( const data::Image &image, const std::string &path, util::istring suffix_override, util::istring dialect )
{
	return write( std::list<data::Image>( 1, image ), path, suffix_override, dialect );
}


bool IOFactory::write( std::list< isis::data::Image > images, const std::string &path, util::istring suffix_override, util::istring dialect )
{
	const std::list<util::istring> formatstack = suffix_override.empty() ?
		getFormatStack(path) : util::stringToList<util::istring>(suffix_override,'.');

	const FileFormatList formatWriter = get().getFileFormatList( formatstack, dialect );

	for( std::list<data::Image>::reference ref :  images ) {
		ref.checkMakeClean();
	}

	if( formatWriter.size() ) {
		for( FileFormatList::const_reference it :  formatWriter ) {
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
			} catch ( const std::exception &e ) {
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
void IOFactory::setProgressFeedback( std::shared_ptr<util::ProgressFeedback> feedback )
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
