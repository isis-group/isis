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
#define BOOST_FILESYSTEM_VERSION 3 
#include <boost/filesystem/path.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace isis
{
enum LogLevel {error = 1, warning, notice, info, verbose_info};
namespace util
{

/**
 * Wrapper to mark the "Subject" in a logging message.
 * Use this to mark the a volatile part of a logging message.
 * eg. \code LOG(Debug,info) << "Loading File " << MSubject(filename); \endcode
 * This will then be ignored when looking for repeating log-messages or can be used for text highlighting.
 */
class MSubject : public std::string
{
public:
	template<typename T> MSubject( const T &cont ) {
		std::ostringstream text;
		text << cont;
		assign( text.str() );
	}
	MSubject( const boost::filesystem::path &cont ) {
		std::ostringstream text;
		text << cont.native();
		assign( text.str() );
	}
};

const char *logLevelName( LogLevel level );

template<class MODULE> class Log;
class Message;

///Abstract base class for message output handlers
class MessageHandlerBase
{
	static LogLevel m_stop_below;
protected:
	MessageHandlerBase( LogLevel level ): m_level( level ) {}
	virtual ~MessageHandlerBase() {}
public:
	LogLevel m_level;
	virtual void commit( const Message &msg ) = 0;
	/**
	 * Set loglevel below which the system should stop the process.
	 * Sets the severity at which the OS should send a SIGSTOP to the process to halt it.
	 * This can be usefull for debugging usage.
	 * Example: \code
	 * util::DefaultMsgPrint::stopBelow(warning); //anything more severe than warning will raise SIGSTOP
	 * ...
	 * LOG(Runtime,error) << "This is realy bad.. we should stop and debug .."; //here for example
	 * \endcode
	 * \note available only on UNIX-Systems
	 * \note ignored on release builds
	 */
	static void stopBelow( LogLevel );
	bool requestStop( LogLevel _level );
};

class Message: public std::ostringstream
{
	std::weak_ptr<MessageHandlerBase> commitTo;
public:
	std::string m_object, m_module;
	boost::filesystem::path m_file;
	std::list<std::string> m_subjects;
	boost::posix_time::ptime m_timeStamp;
	int m_line;
	LogLevel m_level;
	Message( std::string object, std::string module, std::string file, int line, LogLevel level, std::weak_ptr<MessageHandlerBase> _commitTo );
	Message( const Message &src );
	~Message();
	std::string merge()const;
	std::string strTime()const;
	template<typename T> Message &operator << (const T& val ) {
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

/**
 * Default message output class.
 * Will print any issued message to the given output stream in the format:  "LOG_MODULE_NAME:LOG_LEVEL_NAME[LOCATION] MESSAGE"
 * The default output stream is std::cout. But can be set using setStream.
 * Location is the calling Object/Method if compiled without debug infos (NDEBUG is set) or FILENAME:LINE_NUMER if compiled with debug infos.
 */
class DefaultMsgPrint : public MessageHandlerBase
{
protected:
	static std::ostream *o;
#ifdef NDEBUG
	static const int max_age = 500;
#else
	static const int max_age = 0;
#endif
	std::list<std::pair<boost::posix_time::ptime, std::string> > last;

public:
	DefaultMsgPrint( LogLevel level ): MessageHandlerBase( level ) {}
	virtual ~DefaultMsgPrint() {}
	void commit( const Message &mesg );
	static void setStream( std::ostream &_o );
};

}
}
#endif //MESSAGE_H
