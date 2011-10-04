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
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace isis
{
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
enum LogLevel {error = 1, warning, info, verbose_info};
namespace util
{

class MSubject : public std::string
{
public:
	template<typename T> MSubject( const T &cont ) {
		std::ostringstream text;
		text << cont;
		assign( text.str() );
	}
};

/// @cond _internal
namespace _internal
{

const char *logLevelNames( LogLevel level );

template<class MODULE> class Log;
class Message;

class MessageHandlerBase
{
	static LogLevel m_stop_below;
protected:
	MessageHandlerBase( LogLevel level ): m_level( level ) {}
	virtual ~MessageHandlerBase() {}
public:
	LogLevel m_level;
	virtual void commit( const Message &msg ) = 0;
	static void stopBelow( LogLevel = error );
	bool requestStop( LogLevel _level );
};

class Message: public std::ostringstream
{
	boost::weak_ptr<MessageHandlerBase> commitTo;
public:
	std::string m_object, m_module;
	boost::filesystem::path m_file;
	std::list<std::string> m_subjects;
	boost::posix_time::ptime m_timeStamp;
	int m_line;
	LogLevel m_level;
	Message( std::string object, std::string module, std::string file, int line, LogLevel m_level, boost::weak_ptr<MessageHandlerBase> commitTo );
	Message( const Message &src );
	~Message();
	std::string merge()const;
	std::string strTime()const;
	template<typename T> Message &operator << ( T val ) {
		*( ( std::ostringstream * )this ) << val;
		return *this;
	}
	Message &operator << ( const MSubject &subj ) {
		m_subjects.push_back( subj );
		*( ( std::ostringstream * )this ) << "{s}";
		return *this;
	}
	bool shouldCommit()const;
};
}
/// @endcond

/**
 * Default message output class.
 * Will print any issued message to the given output stream in the format:  "LOG_MODULE_NAME:LOG_LEVEL_NAME[LOCATION] MESSAGE"
 * The default output stream is std::cout. But can be set using setStream.
 * Location is the calling Object/Method if compiled without debug infos (NDEBUG is set) or FILENAME:LINE_NUMER if compiled with debug infos.
 */
class DefaultMsgPrint : public _internal::MessageHandlerBase
{
protected:
	static std::ostream *o;
	std::string last;
public:
	DefaultMsgPrint( LogLevel level ): _internal::MessageHandlerBase( level ) {}
	virtual ~DefaultMsgPrint() {}
	void commit( const _internal::Message &mesg );
	static void setStream( std::ostream &_o );
};

}
/** @} */
}
#endif //MESSAGE_H
