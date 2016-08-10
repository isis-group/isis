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

class QMessage:public isis::util::Message
{
public:
	QMessage(const QMessage &src);
	QMessage(const isis::util::Message &src);
	QDateTime getTimestamp();
	QString merge();
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
