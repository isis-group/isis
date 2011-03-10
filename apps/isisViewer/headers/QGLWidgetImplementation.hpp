#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#include <QtOpenGL/QGLWidget>
#include "ViewerCore.hpp"
#include <iostream>
#include <DataStorage/chunk.hpp>

namespace isis {
namespace viewer {
	

class QGLWidgetImplementation : public QGLWidget
{
	Q_OBJECT
public:
	QGLWidgetImplementation( ViewerCore* core, QWidget* parent = 0); 
	
	void paint();
	void initializeGL();
	
	
private:
	boost::shared_ptr<ViewerCore> m_ViewerCore;
	GLuint m_TextureID;

public Q_SLOTS:

	protected:
	virtual void mouseMoveEvent(QMouseEvent* e );
	

	
protected:
Q_SIGNALS:
	void redraw();
	
	
	
private:
	void connectSignals();
		
};



}}//end namespace

#endif