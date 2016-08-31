#ifndef QDEFAULTMESSAGEPRINT_HPP
#define QDEFAULTMESSAGEPRINT_HPP

#include "../../util/message.hpp"
#include <QString>
#include <QObject>
#include <QDateTime>

namespace isis
{
namespace qt5
{

class LogEvent
{
public:
	explicit LogEvent(const util::Message &msg);
	QString merge();
	std::string m_object, m_module;
	boost::filesystem::path m_file;
	std::list<std::string> m_subjects;
	QDateTime m_timeStamp;
	int m_line;
	LogLevel m_level;
	QString m_unformatted_msg;
};

class LogEventList : public std::list<LogEvent> {};


class QDefaultMessageHandler : public QObject, public util::MessageHandlerBase
{
	Q_OBJECT
public:
Q_SIGNALS:
	void commitMessage( qt5::LogEvent message );

public:
	virtual void commit( const util::Message &msg );
	void qmessageBelow( LogLevel level );
	   QDefaultMessageHandler( LogLevel level );
	virtual ~QDefaultMessageHandler();
	const LogEventList &getMessageList() const;
private:
	LogLevel m_LogEventLogLevel;

};

}
}


#endif
