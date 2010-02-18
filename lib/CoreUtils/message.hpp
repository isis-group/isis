//
// C++ Interface: message
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef MESSAGE_H
#define MESSAGE_H

#include <sstream>
#include <string>
#include <ctime>
#include <list>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/weak_ptr.hpp>

namespace isis{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

class MSubject : public std::string{
	public:
		template<typename T> MSubject(const T& cont) {
			std::ostringstream text;
			text << cont;
			assign(text.str());
		}
};

enum LogLevel{error=1,warning,info,verbose_info};
static const char* LogLevelNames[]={"no_log","error","warning","info","verbose"};

/// @cond _internal
namespace _internal{

template<class MODULE> class Log;
class Message;

class MessageHandlerBase{
	static LogLevel m_stop_below;
protected:
	MessageHandlerBase(LogLevel level):m_level(level){}
	virtual ~MessageHandlerBase(){}
public:
	LogLevel m_level;
	virtual void commit(const Message &msg)=0;
	static void stopBelow(LogLevel =error);
	bool requestStop(LogLevel _level);
};

class Message: public std::ostringstream{
	boost::weak_ptr<MessageHandlerBase> commitTo;
public:
	std::string m_object,m_module;
	boost::filesystem::path m_file;
	std::list<std::string> m_subjects;
	time_t m_timeStamp;
	int m_line;
	LogLevel m_level;
	Message(std::string object,std::string module,std::string file,int line,LogLevel m_level,boost::weak_ptr<MessageHandlerBase> commitTo);
	Message(const Message &src);
	~Message();
	std::string merge()const;
	std::string strTime()const;
	template<typename T> Message &operator << (T val){
		*((std::ostringstream*)this) << val;
		return *this;
	}
	Message &operator << (const MSubject &subj){
		m_subjects.push_back(subj);
		*((std::ostringstream*)this) << "{s}";
		return *this;
	}
	bool shouldCommit()const;
};
}
/// @endcond

class DefaultMsgPrint : public _internal::MessageHandlerBase {
protected:
	static std::ostream *o;
public:
	DefaultMsgPrint(LogLevel level):_internal::MessageHandlerBase(level){}
	void commit(const _internal::Message &mesg);
	static void setStream( std::ostream &_o);
};

template<class MODULE>class MessageQueue : public _internal::MessageHandlerBase {
	std::list<_internal::Message> msg;
public:
	MessageQueue(LogLevel level):msg(),_internal::MessageHandlerBase(level){}
	~MessageQueue(){
		for(std::list<_internal::Message>::iterator i=msg.begin();i!=msg.end();i++){
		std::cout << i->str() << std::endl;
		}
	}
	void commit(const _internal::Message &mesg){
		std::cout << "Flushing " << mesg.str() << std::endl;
		msg.push_back(mesg);//@needs to be mutexed
	}
};

}
/** @} */
}
#endif //MESSAGE_H
