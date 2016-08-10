#include "qdefaultmessageprint.hpp"
#include "../../util/singletons.hpp"
#include "../../data/image.hpp"
#include <QMessageBox>

isis::qt5::QMessage::QMessage(const isis::util::Message& src): Message(src){}
isis::qt5::QMessage::QMessage(const isis::qt5::QMessage& src): Message(src){}


QDateTime isis::qt5::QMessage::getTimestamp(){
	QDateTime ret;
	ret.setTime_t(m_timeStamp);
	return ret;
}

QString isis::qt5::QMessage::merge()
{
	QString ret=QString::fromStdString(str());

	for(const auto &subj:m_subjects)
		ret.replace(QString("{o}"),QString::fromStdString(subj));
	return  ret;
}

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
	QMessage qMessage(msg);
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

		std::stringstream text;
		text << util::logLevelName( msg.m_level ) << " in " << qMessage.m_file << ":" << qMessage.m_line;
		msgBox.setWindowTitle( QString("%1 (%2)").arg(qMessage.m_module.c_str()).arg(qMessage.getTimestamp().toString()) );
		msgBox.setText( text.str().c_str() );
		msgBox.setInformativeText( qMessage.merge() );
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
