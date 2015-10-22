#ifndef QDEFAULTMESSAGEPRINT_HPP
#define QDEFAULTMESSAGEPRINT_HPP

#include "util/message.hpp"
#include <QString>
#include <QObject>

namespace isis
{
namespace qt5
{

class QMessage
{
public:
	std::string m_object, m_module;
	boost::filesystem::path m_file;
	std::list<std::string> m_subjects;
	boost::posix_time::ptime m_timeStamp;
	int m_line;
	LogLevel m_level;
	std::string message;
	std::string time_str;
};

class QMessageList : public std::list<QMessage> {};


class QDefaultMessagePrint : public QObject, public util::MessageHandlerBase
{
	Q_OBJECT
public:
Q_SIGNALS:
	void commitMessage( qt5::QMessage message );

public:
	virtual void commit( const util::Message &msg );
	void qmessageBelow( LogLevel level );
	QDefaultMessagePrint( LogLevel level );
	virtual ~QDefaultMessagePrint();
	const QMessageList &getMessageList() const;
private:
	LogLevel m_QMessageLogLevel;

};

}
}


#endif