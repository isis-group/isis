#include "qdefaultmessageprint.hpp"
#include "../../util/singletons.hpp"
#include "../../data/image.hpp"
#include <QMessageBox>

isis::qt5::LogEvent::LogEvent(const isis::util::Message& msg)
{
	m_file = msg.m_file;
	m_level = msg.m_level;
	m_line = msg.m_line;
	m_module = msg.m_module;
	m_object = msg.m_object;
	m_subjects = msg.m_subjects;
	m_timeStamp.setTime_t(msg.m_timeStamp);
	m_unformatted_msg = QString::fromStdString(msg.str());
}

QString isis::qt5::LogEvent::merge()
{
	QString ret=m_unformatted_msg;

	for(const auto &subj:m_subjects)
		ret.replace(QString("{o}"),QString::fromStdString(subj));
	return  ret;
}


isis::qt5::QDefaultMessagePrint::QDefaultMessagePrint( isis::LogLevel level )
	: MessageHandlerBase( level ),
	  m_LogEventLogLevel( isis::error )
{}

void isis::qt5::QDefaultMessagePrint::qmessageBelow ( isis::LogLevel level )
{
	m_LogEventLogLevel = level;
}

void isis::qt5::QDefaultMessagePrint::commit( const isis::util::Message &msg )
{
	LogEvent qMessage(msg);
	util::Singletons::get<LogEventList, 10>().push_back( qMessage );
	commitMessage( qMessage );

	if( m_LogEventLogLevel > msg.m_level ) {
		QMessageBox msgBox;
		std::string level;

		switch( msg.m_level ) {
		case isis::verbose_info:
		case isis::info:
		case isis::notice:
			msgBox.setIcon( QMessageBox::Information );
			break;
		case isis::warning:
			msgBox.setIcon( QMessageBox::Warning );
			break;
		case isis::error:
			msgBox.setIcon( QMessageBox::Critical );
			break;
		}

 		std::stringstream text;
 		text << util::logLevelName( msg.m_level ) << " in " << qMessage.m_file << ":" << qMessage.m_line;
		msgBox.setWindowTitle( QString("%1 (%2)").arg(qMessage.m_module.c_str()).arg(qMessage.m_timeStamp.toString()) );
 		msgBox.setText( text.str().c_str() );
		msgBox.setInformativeText( qMessage.merge() );
 		msgBox.exec();
	}
}

isis::qt5::QDefaultMessagePrint::~QDefaultMessagePrint()
{

}
const isis::qt5::LogEventList &isis::qt5::QDefaultMessagePrint::getMessageList() const
{
	return util::Singletons::get<LogEventList, 10>();
}
