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

enum LogLevel{error=0,warning,info,verbose_info};

/// @cond _internal
namespace _internal{

template<class MODULE> class Log;
class Message;

class MessageHandlerBase{
	static LogLevel m_stop_below;
protected:
	MessageHandlerBase(LogLevel _level):level(_level){}
	virtual ~MessageHandlerBase(){}
public:
	LogLevel level;
	virtual void commit(const Message &msg)=0;
	static void stopBelow(LogLevel =error);
	bool requestStop(LogLevel _level);
};

class Message: public std::ostringstream{
public:
	std::string object,file;
	std::list<std::string> subjects;
	time_t timeStamp;
	int line;
	LogLevel level;
	MessageHandlerBase *commitTo;
	Message(std::string object,std::string file,int line,LogLevel level,MessageHandlerBase *commitTo);
	Message(const Message &src);
	std::string merge()const;
	std::string strTime()const;
	template<typename T> Message &operator << (T val){
		if(commitTo)
		*((std::ostringstream*)this) << val;
		return *this;
	}
	Message &operator << (const MSubject &subj){
		subjects.push_back(subj);
		*((std::ostringstream*)this) << "{s}";
		return *this;
	}
  
	Message &operator << (Message & (*op)(Message& os)){
		return (*op)(*this);
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

class DefaultMsgPrintNeq : public DefaultMsgPrint {
	std::string last;
public:
	DefaultMsgPrintNeq(LogLevel level):DefaultMsgPrint(level){}
	void commit(const _internal::Message &mesg);
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

namespace std{
	isis::util::_internal::Message& endl(isis::util::_internal::Message& __os);
}

#endif //MESSAGE_H
