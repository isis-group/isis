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

namespace isis{ namespace util{
	
using _internal::Message;
using ::std::string;


void _internal::MessageHandlerBase::stopBelow(LogLevel stop){
	m_stop_below=stop;
}

bool _internal::MessageHandlerBase::requestStop(LogLevel _level){
	if(m_stop_below>_level)
		return kill(getpid(),SIGTSTP) == 0;
	else 
		return false;
}

string Message::strTime()const{
	char buffer[11];
	tm r = {0};
	strftime(buffer, sizeof(buffer), "%X", localtime_r(&timeStamp, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	::std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000);
	return result;
}

Message::Message(string _object,string _file,int _line,LogLevel _level,MessageHandlerBase *_commitTo)
:object(_object),file(_file),line(_line),level(_level),commitTo(_commitTo){
	time(&timeStamp);
}
Message::Message(const Message &src)
: ::std::ostringstream(src.str()),object(src.object),file(src.file),subjects(src.subjects),timeStamp(src.timeStamp),line(src.line),commitTo(src.commitTo)
{}
Message::~Message()
{
	if(shouldCommit()){
		commitTo->commit(*this);
		str("");
		clear();
		commitTo->requestStop(level);
	}
}

  
string Message::merge()const{
	string ret(str());
	size_t found=string::npos;
	std::list<std::string>::const_iterator subj=subjects.begin();
	if((found=ret.find("{o}"))!=string::npos)
		ret.replace(found,3,object);
	found=0;
	while((found=ret.find("{s}",found))!=string::npos)
		ret.replace(found,3,*(subj++));
	return ret;
}

void DefaultMsgPrint::commit(const Message &mesg){
	*o << LogLevelNames[mesg.level] << " [" << mesg.file.leaf() << ":" << mesg.line << "|" << mesg.object << "]\t" <<
mesg.merge() << std::endl;
}

void DefaultMsgPrint::setStream(::std::ostream &_o){
	o = &_o;
}

bool Message::shouldCommit()const{
	if(commitTo)
		return (commitTo->level >= level);
	else return false;
}

::std::ostream *DefaultMsgPrint::o=&::std::cout;
LogLevel _internal::MessageHandlerBase::m_stop_below=error;


}}

/// @cond _hidden
namespace std{
// @todo obsolete - still there for backward compatibility
isis::util::_internal::Message& endl(isis::util::_internal::Message& __os) {return __os;}
}
/// @endcond
