#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#include <QtOpenGL/QGLWidget>
#include "ViewerCoreBase.hpp"
#include <iostream>

namespace isis {
namespace viewer {
	

class QGLWidgetImplementation : public QGLWidget
{

public:
	QGLWidgetImplementation( ViewerCoreBase* core, QWidget* parent = 0); 
private:
	boost::shared_ptr<ViewerCoreBase> m_ViewerCore;

	bool m_Valid;
	
	
	
		
};




}}//end namespace

#endif