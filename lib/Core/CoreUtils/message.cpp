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
#include <sys/types.h>

#include <boost/date_time/posix_time/posix_time.hpp> //needed to print the timestamp
#include <sstream>

#ifndef WIN32
#include <signal.h>
#endif

#ifdef HAVE_CURSES
#include <curses.h>
#include <term.h>
#endif // HAVE_CURSES

namespace isis
{
namespace util
{
namespace _internal
{
struct IsEqualMsg {
	std::string newMsg;
	bool operator()( const std::pair<boost::posix_time::ptime, std::string>& ms ) {return ms.second == newMsg;}
};
}
const char *logLevelName( LogLevel level )
{
	switch( level ) {
	case error:
		return "error";
	case warning:
		return "warning";
	case notice:
		return "notice";
	case info:
		return "info";
	case verbose_info:
		return "verbose";
	}

	return "//no_log//";
}

void MessageHandlerBase::stopBelow( LogLevel stop )
{
#ifdef WIN32
	LOG( Debug, error ) << "Sorry stopping is not supported on Win32";
	return;
#endif
#ifdef NDEBUG
	LOG( Debug, error ) << "Wont apply stopping because NDEBUG is set";
	return;
#endif
	m_stop_below = stop;
}

bool MessageHandlerBase::requestStop( LogLevel _level )
{
	if ( m_stop_below > _level ) {
#ifdef WIN32
		return false;
#else
		return kill( getpid(), SIGTSTP ) == 0;
#endif
	} else
		return false;
}

std::string Message::strTime()const
{
	return boost::posix_time::to_simple_string( m_timeStamp );
}

Message::Message( std::string object, std::string module, std::string file, int line, LogLevel level, std::weak_ptr<MessageHandlerBase> _commitTo )
	: commitTo( _commitTo ),
	  m_object( object ),
	  m_module( module ),
	  m_file( file ),
	  m_timeStamp( boost::posix_time::microsec_clock::universal_time() ),
	  m_line( line ),
	  m_level( level )
{}

Message::Message( const Message &src ) //we need a custom copy-constructor, because the copy-contructor of ostringstream is private
	: std::ostringstream( src.str() ),
	  commitTo( src.commitTo ),
	  m_object( src.m_object ),
	  m_module( src.m_module ),
	  m_file( src.m_file ),
	  m_subjects( src.m_subjects ),
	  m_timeStamp( src.m_timeStamp ),
	  m_line( src.m_line ),
	  m_level( src.m_level )
{}

Message::~Message()
{
	if ( shouldCommit() ) {
		commitTo.lock()->commit( *this );
		str( "" );
		clear();
		commitTo.lock()->requestStop( m_level );
	}
}


std::string Message::merge(const std::string color_code)const
{
	const std::string reset_code(color_code.empty()? "":"\033[0m");
	const std::string s_prefix(color_code.empty()? "\"":"\x1B[1m");
	const std::string s_suffix(color_code.empty()? std::string("\""):reset_code+color_code);
	
	std::string ret( str() );
	size_t found = std::string::npos;
	std::list<std::string>::const_iterator subj = m_subjects.begin();

	if ( ( found = ret.find( "{o}" ) ) != std::string::npos )
		ret.replace( found, 3, m_object );

	found = 0;

	while ( ( found = ret.find( "{s}", found ) ) != std::string::npos )
		ret.replace( found, 3, s_prefix + * ( subj++ ) + s_suffix );

	return  color_code+ret+reset_code;
}

bool Message::shouldCommit()const
{
	if( str().empty() )
		return false;

	const std::shared_ptr<MessageHandlerBase> buff( commitTo.lock() );

	if ( buff )
		return ( buff->m_level >= m_level );
	else return false;
}

LogLevel MessageHandlerBase::m_stop_below = error;

DefaultMsgPrint::DefaultMsgPrint(LogLevel level): MessageHandlerBase( level ), istty(isatty(fileno(stderr))) {}

void DefaultMsgPrint::commit( const Message &mesg )
{
	if(istty)
		commit_tty(mesg);
	else
		commit_pipe(mesg);
}


void DefaultMsgPrint::commit_tty(const Message& mesg)
{
	//first remove everything which is to old anyway
	std::list< std::pair<boost::posix_time::ptime, std::string> >::iterator begin = last.begin();
	static const boost::posix_time::millisec dist( max_age );

	const char *color_code="";
	
#ifdef HAVE_CURSES
	static int err_dummy;
	static int is_no_term=setupterm(0,fileno(stderr),&err_dummy);
	
	// terminal color codes
	const char red_code[]="\x1B[31m";
	const char yellow_code[]="\x1B[33m";
	const char green_code[]="\x1B[32m";
	const char white_code[]="\x1B[37m";
	const char norm_code[]="\x1B[0m";
	
	if(!is_no_term && cur_term && max_colors>=8)
		switch(mesg.m_level){
			case error:color_code=red_code;
			case warning:color_code=yellow_code;break;
			case notice:color_code=green_code;break;
			case info:color_code=white_code;break;
			default:color_code=norm_code;break;
		}
#endif //HAVE_CURSES

	while( begin != last.end() && begin->first + dist < mesg.m_timeStamp ) {
		begin++;
		last.pop_front();
	}

	const _internal::IsEqualMsg isEqual = {mesg.str()};

	begin = std::find_if( last.begin(), last.end(), isEqual );

	if( begin == last.end() ) { // its not in the list of the last xxx milliseconds - so print it
#ifndef NDEBUG //if with debug-info
		fprintf(stderr,"%s:%s[%s:%d]%s\n",
				mesg.m_module.c_str(),logLevelName( mesg.m_level ),mesg.m_file.leaf().c_str(),mesg.m_line,mesg.merge(color_code).c_str()
		);
#else
		fprintf(stderr,"%s:%s[%s]%s\n",
				mesg.m_module.c_str(),logLevelName( mesg.m_level ),mesg.m_object.c_str(),mesg.merge(color_code).c_str()
		);
#endif //NDEBUG
	} else {
		last.erase( begin ); // it was in the list - remove it
	}

	last.push_back( std::make_pair( mesg.m_timeStamp, mesg.str() ) ); // put new message on the top
}

void DefaultMsgPrint::commit_pipe(const Message& mesg)
{
	fprintf(stderr,"%s:%s %s [%s -- %s:%d]\n",
			mesg.m_module.c_str(),util::logLevelName( mesg.m_level ),mesg.merge("").c_str(), mesg.strTime().c_str(),mesg.m_file.leaf().c_str(),mesg.m_line);
}

}
}
