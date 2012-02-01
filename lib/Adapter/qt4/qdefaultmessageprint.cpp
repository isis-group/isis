#include "qdefaultmessageprint.hpp"
#include <CoreUtils/singletons.hpp>

isis::qt4::QDefaultMessagePrint::QDefaultMessagePrint( isis::LogLevel level ): MessageHandlerBase( level ) {}


void isis::qt4::QDefaultMessagePrint::commit( const isis::util::_internal::Message &msg )
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
}

isis::qt4::QDefaultMessagePrint::~QDefaultMessagePrint()
{

}
const isis::qt4::QMessageList &isis::qt4::QDefaultMessagePrint::getMessageList() const
{
	return util::Singletons::get<QMessageList, 10>();
}
