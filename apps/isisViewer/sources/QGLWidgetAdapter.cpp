#include "QGLWidgetAdapter.hpp"

namespace isis {
namespace viewer {


QGLWidgetAdapter::QGLWidgetAdapter( boost::shared_ptr<ViewerCoreBase> core )
	: m_Valid( false ),
	m_ViewerCore( core )
{
	
}

void QGLWidgetAdapter::setParent( QWidget* parent ) 
{
	m_Target = boost::shared_ptr<QGLWidgetImpl>( new QGLWidgetImpl( parent ) );
	m_Valid = true;
}


void QGLWidgetAdapter::paint() const
{
	std::cout << "gsjhgsdf" << std::endl;
	m_Target->initializeGL();
}

}} // end namespace