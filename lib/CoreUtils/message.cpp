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
#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> //we need the to_string functions for the automatic conversion

#ifndef WIN32
#include <signal.h>
#endif

namespace isis
{
namespace util
{
namespace _internal
{
const char* LogLevelNames[] = {"no_log", "error", "warning", "info", "verbose"};

void MessageHandlerBase::stopBelow( LogLevel stop )
{
	m_stop_below = stop;
}

bool MessageHandlerBase::requestStop( LogLevel _level )
{
	if ( m_stop_below > _level ){
#ifdef WIN32
		LOG(Debug,error) << "Sorry stopping is not supportted on Win32";
		return false;
#else
		return kill( getpid(), SIGTSTP ) == 0;
#endif
	} else
		return false;
}

std::string Message::strTime()const
{
	return boost::posix_time::to_simple_string(m_timeStamp);
}

Message::Message( std::string object, std::string module, std::string file, int line, LogLevel level, boost::weak_ptr<MessageHandlerBase> _commitTo )
: commitTo( _commitTo ), m_object( object ), m_module( module ), m_file( file ), m_line( line ), m_level( level ),m_timeStamp(boost::posix_time::second_clock::universal_time())
{
}
Message::Message( const Message &src ) //we need a custom copy-constructor, because the copy-contructor of ostringstream is private
		: std::ostringstream( src.str() ),
		commitTo( src.commitTo ), m_object( src.m_object ), m_module( src.m_module ), m_file( src.m_file ), m_subjects( src.m_subjects ), m_timeStamp( src.m_timeStamp ), m_line( src.m_line )
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


std::string Message::merge()const
{
	std::string ret( str() );
	size_t found = std::string::npos;
	std::list<std::string>::const_iterator subj = m_subjects.begin();

	if ( ( found = ret.find( "{o}" ) ) != std::string::npos )
		ret.replace( found, 3, m_object );

	found = 0;

	while ( ( found = ret.find( "{s}", found ) ) != std::string::npos )
		ret.replace( found, 3, std::string( "\"" ) + *( subj++ ) + "\"" );

	return ret;
}

bool Message::shouldCommit()const
{
	const boost::shared_ptr<MessageHandlerBase> buff( commitTo.lock() );

	if ( buff )
		return ( buff->m_level >= m_level );
	else return false;
}

LogLevel MessageHandlerBase::m_stop_below = error;


} //namespace _internal

std::ostream *DefaultMsgPrint::o = &::std::cerr;
void DefaultMsgPrint::commit( const _internal::Message &mesg )
{
	*o << mesg.m_module << ":" << _internal::LogLevelNames[mesg.m_level]
#ifndef NDEBUG //if with debug-info
	<< "[" << mesg.m_file.leaf() << ":" << mesg.m_line << "] " //print the file and the line
#else
	<< "[" << mesg.m_object << "] " //print the object/method
#endif //NDEBUG
	<< mesg.merge()
	<< std::endl;
}

void DefaultMsgPrint::setStream( ::std::ostream &_o )
{
	o->flush();
	o = &_o;
}

}
}
