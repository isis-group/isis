/*
 *  message.h
 *  log_concept
 *
 *  Created by Enrico Reimer on 04.05.08.
 *  Copyright: See COPYING file that comes with this distribution
 *
 */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <sstream>
#include <string>
#include <ctime>
#include <list>
#include <iostream>

namespace isis{

template<class MODULE> class Log;
class Message;

class MessageHandlerBase{
protected:
  MessageHandlerBase(unsigned short _level):level(_level){}
  virtual ~MessageHandlerBase(){}
public:
  unsigned short level;
  virtual void commit(const Message &msg)=0; 
};

class MSubject : public std::string{
public:
  template<typename T> MSubject(const T& cont) : std::string(){ 
    std::ostringstream text;
    text << cont; 
    assign(text.str());
  }
};

class Message: public std::ostringstream{
public:
  std::string object,file;
  std::list<std::string> subjects;
  time_t timeStamp;
  int line;
  unsigned short level;
  MessageHandlerBase *commitTo;
  Message(std::string object,std::string file,int line,unsigned short level,MessageHandlerBase *commitTo);
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

class DefaultMsgPrint : public MessageHandlerBase {
protected:
	static std::ostream *o;
public:
	DefaultMsgPrint(unsigned short level):MessageHandlerBase(level){}
	void commit(const Message &mesg);
	static void setStream( std::ostream &_o);
};

class DefaultMsgPrintNeq : public DefaultMsgPrint {
  std::string last;
public:
  DefaultMsgPrintNeq(unsigned short level):DefaultMsgPrint(level){}
  void commit(const Message &mesg);
};


template<class MODULE>class MessageQueue : public MessageHandlerBase {
  std::list<Message> msg;
public:
  MessageQueue(unsigned short level):msg(),MessageHandlerBase(level){}
  ~MessageQueue(){
    for(std::list<Message>::iterator i=msg.begin();i!=msg.end();i++){
      std::cout << i->str() << std::endl;
    }
  }
  void commit(const Message &mesg){
      std::cout << "Flushing " << mesg.str() << std::endl;
      msg.push_back(mesg);//@needs to be mutexed
  }
};

}

namespace std{
  isis::Message& endl(isis::Message& __os);
}

#include "log.hpp"

#endif //MESSAGE_H
