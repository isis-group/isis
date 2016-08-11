#include "qdefaultmessageprint.hpp"
#include "../../util/singletons.hpp"
#include "../../data/image.hpp"
#include <QMessageBox>

isis::qt5::QDefaultMessagePrint::QDefaultMessagePrint( isis::LogLevel level )
	: MessageHandlerBase( level ),
	  m_QMessageLogLevel( isis::error )
{}

void isis::qt5::QDefaultMessagePrint::qmessageBelow ( isis::LogLevel level )
{
	m_QMessageLogLevel = level;
}

void isis::qt5::QDefaultMessagePrint::commit( const isis::util::Message &msg )
{
	QMessage qMessage;
	qMessage.m_file = msg.m_file;
	qMessage.m_level = msg.m_level;
	qMessage.m_line = msg.m_line;
	qMessage.m_module = msg.m_module;
	qMessage.m_object = msg.m_object;
	qMessage.m_subjects = msg.m_subjects;
	qMessage.m_timeStamp = msg.m_timeStamp;
	qMessage.message = msg.merge("");
	qMessage.time_str = msg.strTime();
	util::Singletons::get<QMessageList, 10>().push_back( qMessage );
	commitMessage( qMessage );

	if( m_QMessageLogLevel > msg.m_level ) {
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

		std::stringstream windowTitle;
		std::stringstream text;
		windowTitle << qMessage.m_module << " (" << qMessage.time_str << ")";
		text << util::logLevelName( msg.m_level ) << " in " << qMessage.m_file << ":" << qMessage.m_line;
		msgBox.setWindowTitle( windowTitle.str().c_str() );
		msgBox.setText( text.str().c_str() );
		msgBox.setInformativeText( qMessage.message.c_str() );
		msgBox.exec();
	}
}

isis::qt5::QDefaultMessagePrint::~QDefaultMessagePrint()
{

}
const isis::qt5::QMessageList &isis::qt5::QDefaultMessagePrint::getMessageList() const
{
	return util::Singletons::get<QMessageList, 10>();
}
