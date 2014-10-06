#include "common.hpp"
#include <string>

namespace isis
{
namespace util
{

std::string getLastSystemError()
{
#ifdef WIN32
	std::string ret;
	LPTSTR s;
	DWORD err = GetLastError();

	if( ::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						 NULL,
						 err,
						 0,
						 ( LPTSTR )&s,
						 0,
						 NULL ) == 0 ) { /* failed */
		ret = std::string( "Unknown error " ) + std::to_string( err );
	} else { /* success */
		ret = s;
		ret.resize( ret.rfind( '\r' ) ); //FormatMessage appends a newline
		::LocalFree( s );
	}

	return ret;
#else
	return strerror( errno );
#endif
}
boost::filesystem::path getRootPath(std::list< boost::filesystem::path > sources,bool sorted)
{
	if(!sorted)
		sources.sort();
	sources.erase( std::unique( sources.begin(), sources.end() ), sources.end() );
	
	if( sources.empty() ) {
		LOG( Runtime, error ) << "Failed to get common source";
		return boost::filesystem::path();
	} else if( sources.size() == 1 )
		return *sources.begin();
	else {
		for( boost::filesystem::path & ref : sources )
			ref.remove_filename();
		return getRootPath( sources,true );
	}
}


}
}
