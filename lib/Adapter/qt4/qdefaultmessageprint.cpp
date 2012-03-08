#include "qdefaultmessageprint.hpp"
#include <CoreUtils/singletons.hpp>
#include <QMessageBox>

isis::qt4::QDefaultMessagePrint::QDefaultMessagePrint( isis::LogLevel level )
	: MessageHandlerBase( level ),
	m_QMessageLogLevel( isis::error )
{}


void isis::qt4::QDefaultMessagePrint::commit( const isis::util::Message &msg )
{
	QMessage qMessage;
	qMessage.m_file = msg.m_file;
	qMessage.m_level = msg.m_level;
	qMessage.m_line = msg.m_line;
	qMessage.m_module = msg.m_module;
	qMessage.m_object = msg.m_object;
	qMessage.m_subjects = msg.m_subjects;
	qMessage.m_timeStamp = msg.m_timeStamp;
	qMessage.message = msg.merge();
	qMessage.time_str = msg.strTime();
	util::Singletons::get<QMessageList, 10>().push_back( qMessage );
	commitMessage( qMessage );
	if( m_QMessageLogLevel < msg.m_level ) {
		QMessageBox msgBox;
		switch( msg.m_level ) {
			case isis::info:
			case isis::verbose_info:
			case isis::notice:
				msgBox.setIcon(QMessageBox::Information);
				break;
			case isis::warning:
				msgBox.setIcon( QMessageBox::Warning );
				break;
			case isis::error:
				msgBox.setIcon( QMessageBox::Critical );
				break;
		}
		std::stringstream errorMessage;
		errorMessage << qMessage.m_module << "(" << qMessage.time_str << ") [" << qMessage.m_file << ":" << qMessage.m_line << "] " << qMessage.message;
		msgBox.setText( errorMessage.str().c_str() );
		msgBox.exec();
	}
}

isis::qt4::QDefaultMessagePrint::~QDefaultMessagePrint()
{

}
const isis::qt4::QMessageList &isis::qt4::QDefaultMessagePrint::getMessageList() const
{
	return util::Singletons::get<QMessageList, 10>();
}
