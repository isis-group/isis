#ifndef FILTER_COMMON_ROOT_HPP
#define FILTER_COMMON_ROOT_HPP

#include "../Core/CoreUtils/log_modules.hpp"
#include "../Core/CoreUtils/log.hpp"

namespace isis
{
namespace filter
{

typedef FilterLog Runtime;
typedef FilterDebug Debug;

template<typename HANDLE> void enableLog( LogLevel level )
{
	ENABLE_LOG( Runtime, HANDLE, level );
	ENABLE_LOG( Debug, HANDLE, level );
}

}
}

#endif