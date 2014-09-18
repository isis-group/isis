//
// C++ Interface: log
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef LOG_H
#define LOG_H

#include <string>
#include "message.hpp"
#include "singletons.hpp"
#include <limits.h>

/// @cond _internal
namespace isis
{
namespace util
{
namespace _internal
{

template<class MODULE> class Log
{
	friend class util::Singletons;
	std::shared_ptr<MessageHandlerBase> m_handle;
	static std::shared_ptr<MessageHandlerBase> &getHandle() {
		std::shared_ptr<util::MessageHandlerBase> &handle = Singletons::get < Log<MODULE>, INT_MAX - 1 > ().m_handle;
		return handle;
	}
	Log(): m_handle( new DefaultMsgPrint( notice ) ) {}
public:
	template<class HANDLE_CLASS> static void enable( LogLevel enable ) {
		setHandler( std::shared_ptr<MessageHandlerBase>( enable ? new HANDLE_CLASS( enable ) : 0 ) );
	}
	static void setHandler( std::shared_ptr<MessageHandlerBase> handler ) {
		getHandle() = handler;
	}
	static Message send( const char file[], const char object[], int line, LogLevel level ) {
		std::shared_ptr<util::MessageHandlerBase> &handle = getHandle();
		return Message( object, MODULE::name(), file, line, level, handle );
	}
};

}
}
}
/// @endcond

#define ENABLE_LOG(MODULE,HANDLE_CLASS,set)\
	if(!MODULE::use);else isis::util::_internal::Log<MODULE>::enable<HANDLE_CLASS>(set)

#define LOG(MODULE,LEVEL)\
	if(!MODULE::use);else isis::util::_internal::Log<MODULE>::send(__FILE__,__FUNCTION__,__LINE__,LEVEL)

#define LOG_IF(PRED,MODULE,LEVEL)\
	if(!(MODULE::use && (PRED)));else isis::util::_internal::Log<MODULE>::send(__FILE__,__FUNCTION__,__LINE__,LEVEL)

#endif
