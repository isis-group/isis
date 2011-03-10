#include "QGLWidgetImplementation.hpp"

namespace isis {
namespace viewer {

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCoreBase* core, QWidget* parent )
	: QGLWidget( parent ),
	m_ViewerCore( boost::shared_ptr<ViewerCoreBase>(core) )
{
	
}
	




}} // end namespace