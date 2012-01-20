#include "qdefaultmessageprint.hpp"

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
	commitMessage( qMessage );
}

isis::qt4::QDefaultMessagePrint::~QDefaultMessagePrint()
{

}
