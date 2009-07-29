/*
 *  message.cpp
 *  log_concept
 *
 *  Created by Enrico Reimer on 04.05.08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 */

#include "message.h"
#include <sys/time.h>


std::string Message::strTime()const{
  char buffer[11];
  tm r = {0};
  strftime(buffer, sizeof(buffer), "%X", localtime_r(&timeStamp, &r));
  struct timeval tv;
  gettimeofday(&tv, 0);
  char result[100] = {0};
  std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
  return result;
}

Message::Message(std::string _object,std::string _file,int _line,unsigned short _level,MessageHandlerBase *_commitTo)
  :object(_object),subject(),file(_file),line(_line),level(_level),commitTo(_commitTo){
  time(&timeStamp);
}
Message::Message(const Message &src)
:std::ostringstream(src.str()),object(src.object),subject(src.subject),file(src.file),timeStamp(src.timeStamp),line(src.line),commitTo(src.commitTo)
{}

  
std::string Message::merge()const{
  std::string ret(str());
  size_t found;
  if((found=ret.find("{o}"))!=std::string::npos)
    ret.replace(found,3,object);
  if((found=ret.find("{s}"))!=std::string::npos)
    ret.replace(found,3,subject);
  return ret;
}

void DefaultMsgPrint::commit(const Message &mesg){
  *o << "[" << mesg.strTime() << "|" << mesg.file << "::" << mesg.object << ":" << mesg.line << "]\t" << mesg.merge() << std::endl;
}

void DefaultMsgPrint::setStream(std::ostream &_o){
	o = &_o;
}

void DefaultMsgPrintNeq::commit(const Message &mesg){
  const std::string out(mesg.merge());
  if(last!=out){
    *o << "[" << mesg.strTime() << "|" << mesg.file << "::" << mesg.object << ":" << mesg.line << "]\t" <<  out << std::endl;
    last=out;
  }
}

bool Message::shouldCommit()const{
  return (commitTo && commitTo->level >= level);
}

Message& std::endl(Message& __os) { 
  if(__os.shouldCommit()){
    __os.commitTo->commit(__os);
    __os.str("");
    __os.clear();
  }
  return __os;
}

std::ostream *DefaultMsgPrint::o=&std::cout;
