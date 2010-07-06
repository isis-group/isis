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
#else
#include <dlfcn.h>
#endif
#include <iostream>
#include "CoreUtils/log.hpp"
#include "common.hpp"
#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include "CoreUtils/singletons.hpp"

namespace isis
{
namespace data
{

namespace _internal
{
struct pluginDeleter {
	void *m_dlHandle;
	std::string m_pluginName;
	pluginDeleter( void *dlHandle, std::string pluginName ): m_dlHandle( dlHandle ), m_pluginName( pluginName ) {}
	void operator()( image_io::FileFormat *format ) {
		delete format;
#ifdef WIN32
		if(!FreeLibrary( (HINSTANCE)m_dlHandle ))
#else
		if ( dlclose( m_dlHandle ) != 0 )
#endif
			std::cerr << "Failed to release plugin " << m_pluginName << " (was loaded at " << m_dlHandle << ")";

		//we cannot use LOG here, because the loggers are gone allready
	}
};
}

IOFactory::IOFactory()
{
	findPlugins( std::string( PLUGIN_PATH ) );
}

bool IOFactory::registerFormat( const FileFormatPtr plugin )
{
	if ( !plugin )return false;

	io_formats.push_back( plugin );
	std::list<std::string> suffixes = getSuffixes( plugin );
	LOG( Runtime, info )
			<< "Registering " << ( plugin->tainted() ? "tainted " : "" ) << "io-plugin "
			<< util::MSubject( plugin->name() )
			<< " with " << suffixes.size() << " supported suffixes";
	BOOST_FOREACH( std::string & it, suffixes ) {
		io_suffix[it].push_back( plugin );
	}
	return true;
}

unsigned int IOFactory::findPlugins( const std::string &path )
{
	boost::filesystem::path p( path );

	if ( !exists( p ) ) {
		LOG( Runtime, warning ) << util::MSubject( p.native_file_string() ) << " not found";
		return 0;
	}

	if ( !boost::filesystem::is_directory( p ) ) {
		LOG( Runtime, warning ) << util::MSubject( p.native_file_string() ) << " is no directory";
		return 0;
	}

	LOG( Runtime, info )   << "Scanning " << util::MSubject( p ) << " for plugins";
	boost::regex pluginFilter( std::string( "^" ) + DL_PREFIX + "isisImageFormat_" + "[[:word:]]+" + DL_SUFFIX + "$" );
	unsigned int ret = 0;

	for ( boost::filesystem::directory_iterator itr( p ); itr != boost::filesystem::directory_iterator(); ++itr ) {
		if ( boost::filesystem::is_directory( *itr ) )continue;

		if ( boost::regex_match( itr->path().leaf(), pluginFilter ) ) {
			const std::string pluginName = itr->path().string();
#ifdef WIN32
			HINSTANCE handle=LoadLibrary( pluginName.c_str() ); 
#else
			void *handle = dlopen( pluginName.c_str(), RTLD_NOW );
#endif

			if ( handle ) {
#ifdef WIN32
				image_io::FileFormat* ( *factory_func )() = ( image_io::FileFormat * ( * )() )GetProcAddress( handle, "factory");
#else
				image_io::FileFormat* ( *factory_func )() = ( image_io::FileFormat * ( * )() )dlsym( handle, "factory" );
#endif


				if ( factory_func ) {
					FileFormatPtr io_class( factory_func(), _internal::pluginDeleter( handle, pluginName ) );

					if ( registerFormat( io_class ) )
						ret++;
					else
						LOG( Runtime, error ) << "failed to register plugin " << util::MSubject( pluginName );
				} else {
#ifdef WIN32
					LOG( Runtime, error )
							<< "could not get format factory function from " << util::MSubject( pluginName );
					FreeLibrary( handle );
#else
					LOG( Runtime, error )
							<< "could not get format factory function from " << util::MSubject( pluginName ) << ":" << util::MSubject( dlerror() );
					dlclose( handle );
#endif
				}
			} else
#ifdef WIN32
				LOG( Runtime, error )
				<< "Could not load library " << pluginName;
#else
				LOG( Runtime, error )
				<< "Could not load library " << pluginName << ":" << util::MSubject( dlerror() );
#endif
		} else {
			LOG( Runtime, verbose_info )
					<< "Ignoring " << util::MSubject( itr->path() )
					<< " because it doesn't match " << pluginFilter.str();
		}
	}

	return ret;
}

std::list<std::string> IOFactory::getSuffixes( const FileFormatPtr &reader )
{
	return util::string2list<std::string>( reader->suffixes(), boost::regex( "\\s+" ) );
}

IOFactory &IOFactory::get()
{
	return util::Singletons::get<IOFactory, INT_MAX>();
}

int IOFactory::loadFile( isis::data::ChunkList &ret, const boost::filesystem::path &filename, std::string suffix_override, std::string dialect )
{
	FileFormatList formatReader = getFormatInterface( filename.string(), suffix_override, dialect );
	BOOST_FOREACH( FileFormatList::const_reference it, formatReader ) {
		LOG( ImageIoDebug, info )
				<< "plugin to load file " <<  util::MSubject( filename ) << ": " << it->name()
				<<  ( dialect.empty() ?
					  std::string( "" ) : std::string( " with dialect: " ) + dialect
					);

		try {
			return it->load( ret, filename.string(), dialect );
		} catch ( std::runtime_error &e ) {
			LOG( Runtime, error )
					<< "Failed to load " <<  filename << " using " <<  it->name() << " ( " << e.what() << " )";
		}
	}
	LOG( Runtime, error ) << "No plugin found that is able to load: " << filename
			<< " with the dialect [" << dialect << "]"; //@todo error message missing
	return 0;//no plugin of proposed list could load file
}

IOFactory::FileFormatList IOFactory::getFormatInterface( std::string filename, std::string suffix_override, std::string dialect )
{
	std::string ext;

	if( suffix_override.empty() ) {
		boost::filesystem::path fname( filename );
		ext = boost::filesystem::extension( fname );
	} else ext = suffix_override;

	if ( ext.empty() ) {
		return FileFormatList();
	}

	if ( true == dialect.empty() ) {//give back whole list of plugins for this file extension
		return io_suffix[ext];
	}

	//otherwise sort out by dialect
	FileFormatList reader;

	for ( FileFormatList::const_iterator it = io_suffix[ext].begin(); it != io_suffix[ext].end(); it++ ) {
		if ( std::string::npos != ( *it )->dialects().find( dialect ) ) {
			reader.push_back( *it );
		}
	}

	return reader;
}


data::ImageList IOFactory::load( const std::string &path, std::string suffix_override, std::string dialect )
{
	const boost::filesystem::path p( path );
	ChunkList chunks;
	const int loaded = boost::filesystem::is_directory( p ) ?
					   get().loadPath( chunks, p, suffix_override, dialect ) :
					   get().loadFile( chunks, p, suffix_override, dialect );
	BOOST_FOREACH(data::ChunkList::reference ref, chunks){
		if ( ! ref.hasProperty( "source" ) )
			ref.setProperty( "source", p.string() );
	}
	const data::ImageList images( chunks );
	LOG( Runtime, info )
			<< "Generated " << images.size() << " images out of " << loaded << " chunks from " << ( boost::filesystem::is_directory( p ) ? "directory " : "" ) << p;
	return images;
}

int IOFactory::loadPath( isis::data::ChunkList &ret, const boost::filesystem::path &path, std::string suffix_override, std::string dialect )
{
	int loaded = 0;

	for ( boost::filesystem::directory_iterator itr( path ); itr != boost::filesystem::directory_iterator(); ++itr )  {
		if ( boost::filesystem::is_directory( *itr ) )continue;

		loaded += loadFile( ret, itr->path(), suffix_override, dialect );
	}

	return loaded;
}

bool IOFactory::write( const isis::data::ImageList &images, const std::string &filename, const std::string &dialect )
{
	FileFormatList formatWriter = get().getFormatInterface( filename, "", dialect );
	BOOST_FOREACH( FileFormatList::const_reference it, formatWriter ) {
		LOG( Debug, info )
				<< "plugin to write to " <<  filename << ": " << it->name()
				<<  ( dialect.empty() ?
					  std::string( "" ) :
					  std::string( " using dialect: " ) + dialect
					);

		try {
			it->write( images, filename, dialect );
			LOG( Debug, info ) << images.size() << " images written using " <<  it->name();
			return true;
		} catch ( std::runtime_error &e ) {
			LOG( Runtime, error )
					<< "Failed to write " <<  images.size()
					<< " images using " <<  it->name() << " (" << e.what() << ")";
		}
	}
	LOG( Runtime, error ) << "No plugin found to write to: " << filename; //@todo error message missing
	return false;
}


}
} // namespaces data isis
