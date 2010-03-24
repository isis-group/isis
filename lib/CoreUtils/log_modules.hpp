#ifndef LOG_MOUDLES_HPP_INCLUDED
#define LOG_MOUDLES_HPP_INCLUDED

/// @cond _hidden
namespace isis{ 
	struct CoreLog{static const char* name(){return "Core";};enum {use= _ENABLE_LOG};};
	struct CoreDebug{static const char* name(){return "Core";};enum {use= _ENABLE_DEBUG};};
	
	struct ImageIoLog{static const char* name(){return "ImageIO";};enum {use = _ENABLE_LOG};};
	struct ImageIoDebug{static const char* name(){return "ImageIO";};enum {use= _ENABLE_DEBUG};};
	
	struct DataLog{static const char* name(){return "Data";};enum {use = _ENABLE_LOG};};
	struct DataDebug{static const char* name(){return "Data";};enum {use= _ENABLE_DEBUG};};
}
/// @endcond

#endif //LOG_MOUDLES_HPP_INCLUDED
