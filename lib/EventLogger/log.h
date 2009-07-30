//
// C++ Interface: log
//
// Description:
//
//
// Author:  <>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef LOG_H
#define LOG_H

#include <string>
#include "message.h"
#include "../common.hpp"

namespace isis{

template<class MODULE> class Log{
  string file,object;
  inline static MessageHandlerBase* &handler(){
    static MessageHandlerBase *msg;
    return msg;
  }
public:
  Log(const char *_file,string _object):file(_file),object(_object){ }

  template<class HANDLE_CLASS> static void enable(unsigned short enable){
    Log<MODULE>::handler()= enable ? new HANDLE_CLASS(enable):0;
  }
  Message send(int line,unsigned short level){
    return Message(object,file,line, level,Log<MODULE>::handler());
  }
};
}

#define MAKE_LOG(MODULE)\
isis::Log<MODULE> __logger_ ## MODULE(__FILE__,__PRETTY_FUNCTION__)

#define ENABLE_LOG(MODULE,HANDLE_CLASS,set)\
if(!MODULE::use_rel);else isis::Log<MODULE>::enable<HANDLE_CLASS>(set)

#define LOG(MODULE,LEVEL)\
if(!MODULE::use_rel);else __logger_ ## MODULE.send(__LINE__,LEVEL)


#endif
