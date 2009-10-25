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
#include <boost/scoped_ptr.hpp>
#include "message.hpp"
#include "common.hpp"

/// @cond _internal
namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{namespace _internal{
	
template<class MODULE> class Log{
	const char *file,*object;
	inline static MessageHandlerBase* &handler(){
		static MessageHandlerBase *msg;
		return msg;
	}
public:
	Log(const char *_file,const char *_object):file(_file),object(_object){ }

	template<class HANDLE_CLASS> static void enable(LogLevel enable){
		Log<MODULE>::handler()= enable ? new HANDLE_CLASS(enable):0;
	}
	Message send(int line,LogLevel level)const{
		return Message(std::string(object),std::string(file),line, level,Log<MODULE>::handler());
	}
};

}}
/** @} */
}
/// @endcond

#define MAKE_LOG(MODULE)\
const isis::util::_internal::Log<MODULE>  __logger_ ## MODULE (__FILE__,__PRETTY_FUNCTION__)

#define ENABLE_LOG(MODULE,HANDLE_CLASS,set)\
if(!MODULE::use_rel);else isis::util::_internal::Log<MODULE>::enable<HANDLE_CLASS>(set)

#define LOG(MODULE,LEVEL)\
if(!MODULE::use_rel);else __logger_ ## MODULE.send(__LINE__,LEVEL)


#endif
