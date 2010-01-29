//
// C++ Implementation: message
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "message.hpp"
#include "common.hpp"
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <boost/filesystem/path.hpp>

namespace isis{ namespace util{namespace _internal{

void MessageHandlerBase::stopBelow(LogLevel stop){
	m_stop_below=stop;
}

bool MessageHandlerBase::requestStop(LogLevel _level){
	if(m_stop_below>_level)
		return kill(getpid(),SIGTSTP) == 0;
	else 
		return false;
}

std::string Message::strTime()const{
	char buffer[11];
	tm r = {0};
	strftime(buffer, sizeof(buffer), "%X", localtime_r(&m_timeStamp, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
	return result;
}

Message::Message(std::string object,std::string module,std::string file,int line,LogLevel level,boost::weak_ptr<MessageHandlerBase> _commitTo)
:m_object(object),m_module(module),m_file(file),m_line(line),m_level(level),commitTo(_commitTo)
{
	time(&m_timeStamp);
}
Message::Message(const Message &src) //we need a custom copy-constructor, because the copy-contructor of ostringstream is private
: std::ostringstream(src.str()),
  m_object(src.m_object),m_module(src.m_module),m_file(src.m_file),m_subjects(src.m_subjects),m_timeStamp(src.m_timeStamp),m_line(src.m_line),
  commitTo(src.commitTo)
{}

Message::~Message()
{
	if(shouldCommit()){
		commitTo.lock()->commit(*this);
		str("");
		clear();
		commitTo.lock()->requestStop(m_level);
	}
}

  
std::string Message::merge()const{
	std::string ret(str());
	size_t found=std::string::npos;
	std::list<std::string>::const_iterator subj=m_subjects.begin();
	if((found=ret.find("{o}"))!=std::string::npos)
		ret.replace(found,3,m_object);
	found=0;
	while((found=ret.find("{s}",found))!=std::string::npos)
		ret.replace(found,3, std::string("\"")+*(subj++)+"\"");
	return ret;
}

bool Message::shouldCommit()const{
	if(not commitTo.expired())
		return (commitTo.lock()->m_level >= m_level);
	else return false;
}

LogLevel MessageHandlerBase::m_stop_below=error;


} //namespace _internal

std::ostream *DefaultMsgPrint::o=&::std::cout;
void DefaultMsgPrint::commit(const _internal::Message &mesg){
	*o << mesg.m_module << ":" << LogLevelNames[mesg.m_level]
	   << " [" << mesg.m_file.leaf() << ":" << mesg.m_line << "]\t"
	   << mesg.merge()
	   << std::endl;
}

void DefaultMsgPrint::setStream(::std::ostream &_o){
	o = &_o;
}

}}
