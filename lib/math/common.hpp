#ifndef MATH_COMMON_HPP
#define MATH_COMMON_HPP

#include "../core/util/log.hpp"
#include "../core/util/log_modules.hpp"

namespace isis{
namespace math{

typedef MathDebug Debug;
typedef MathLog Runtime;

/**
 * Set logging level for the namespace util.
 * This logging level will be used for every LOG(Debug,...) and LOG(Runtime,...) within the util namespace.
 * This is affected by by the _ENABLE_LOG and _ENABLE_DEBUG settings of the current compile and won't have an
 * effect on the Debug or Runtime logging if the corresponding define is set to "0".
 * So if you compile with "-D_ENABLE_DEBUG=0" against a library which (for example) was comiled with "-D_ENABLE_DEBUG=1",
 * you won't be able to change the logging level of the debug messages of these library.
 */
template<typename HANDLE> void enableLog( LogLevel level )
{
	ENABLE_LOG( MathLog, HANDLE, level );
	ENABLE_LOG( MathDebug, HANDLE, level );
}
}

}
#endif // MATH_COMMON_HPP
