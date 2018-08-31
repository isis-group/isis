#include "qdefaultmessageprint.hpp"
#include "../../core/singletons.hpp"
#include "../../core/image.hpp"
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


isis::qt5::QDefaultMessageHandler::QDefaultMessageHandler( isis::LogLevel level )
	: MessageHandlerBase( level ),
	  m_LogEventLogLevel( isis::error )
{}

void isis::qt5::QDefaultMessageHandler::qmessageBelow ( isis::LogLevel level )
{
	m_LogEventLogLevel = level;
}

void isis::qt5::QDefaultMessageHandler::commit( const isis::util::Message &msg )
{
	LogEvent qMessage(msg);
	util::Singletons::get<LogEventList, 10>().push_back( qMessage );
	
	if(receivers(SIGNAL(commitMessage( qt5::LogEvent ))))
		commitMessage( qMessage );
	else { // fall back to util::DefaultMsgPrint of nobody is listening
		util::DefaultMsgPrint pr(m_level);
		pr.commit(msg);
	}

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

isis::qt5::QDefaultMessageHandler::~QDefaultMessageHandler(){}
const isis::qt5::LogEventList &isis::qt5::QDefaultMessageHandler::getMessageList() const
{
	return util::Singletons::get<LogEventList, 10>();
}
