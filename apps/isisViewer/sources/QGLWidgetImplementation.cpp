#include "QGLWidgetImplementation.hpp"

namespace isis {
namespace viewer {


QGLWidgetImplementation::QGLWidgetImplementation( boost::shared_ptr<ViewerCoreBase> core )
	: m_Valid( false ),
	m_ViewerCore( core )
{
	
}

void QGLWidgetImplementation::setParent( QWidget* parent ) 
{
// 	m_Target = boost::shared_ptr<QGLWidgetImpl>( new QGLWidgetImpl( parent ) );
	m_Valid = true;
}


void QGLWidgetImplementation::paint() const
{
	std::cout << "gsjhgsdf" << std::endl;
}

}} // end namespace