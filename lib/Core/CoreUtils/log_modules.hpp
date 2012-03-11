#ifndef LOG_MOUDLES_HPP_INCLUDED
#define LOG_MOUDLES_HPP_INCLUDED

/// @cond _internal
namespace isis
{
struct CoreLog {static const char *name() {return "Core";}; enum {use = _ENABLE_LOG};};
struct CoreDebug {static const char *name() {return "CoreDebug";}; enum {use = _ENABLE_DEBUG};};

struct ImageIoLog {static const char *name() {return "ImageIO";}; enum {use = _ENABLE_LOG};};
struct ImageIoDebug {static const char *name() {return "ImageIODebug";}; enum {use = _ENABLE_DEBUG};};

struct DataLog {static const char *name() {return "Data";}; enum {use = _ENABLE_LOG};};
struct DataDebug {static const char *name() {return "DataDebug";}; enum {use = _ENABLE_DEBUG};};
}
/// @endcond

#endif //LOG_MOUDLES_HPP_INCLUDED
