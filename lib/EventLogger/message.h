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
  std::string object,subject,file;
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
    subject = subj;
    *((std::ostringstream*)this) << "{s}";
    return *this;
  }
  
  Message &operator << (Message & (*op)(Message& os)){
    return (*op)(*this);
  }
  bool shouldCommit()const;
};

class MessagePrint : public MessageHandlerBase {
public:
  MessagePrint(unsigned short level):MessageHandlerBase(level){}
  void commit(const Message &mesg);
};

class MessagePrintNeq : public MessageHandlerBase {
  std::string last;
public:
  MessagePrintNeq(unsigned short level):MessageHandlerBase(level){}
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

namespace std{
  Message& endl(Message& __os);
}

  
#include "log.h"

#endif //MESSAGE_H
