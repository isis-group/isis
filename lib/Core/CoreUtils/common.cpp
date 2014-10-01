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
}
}
