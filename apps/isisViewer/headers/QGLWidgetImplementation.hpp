#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#include <QtOpenGL/QGLWidget>
#include "WidgetAdapterBase.hpp"
#include "ViewerCoreBase.hpp"
#include <iostream>

namespace isis {
namespace viewer {
	

class QGLWidgetImplementation : public WidgetAdapterBase
{
	class QGLWidgetImpl : public QGLWidget
	{
		
	};
	
	
	
public:
	QGLWidgetImplementation( boost::shared_ptr<ViewerCoreBase> core);

	void setParent( QWidget* parent );
	
	void paint() const;
	
private:
	bool m_Valid;
	boost::shared_ptr<QGLWidgetImpl> m_Target;
	boost::shared_ptr<ViewerCoreBase> m_ViewerCore;
	
	
	
	
		
};




}}//end namespace

#endif