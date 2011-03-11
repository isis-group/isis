#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#define GL_GLEXT_PROTOTYPES

#include <QtOpenGL/QGLWidget>
#include "ViewerCore.hpp"
#include "GLMemoryManager.hpp"
#include <iostream>
#include <DataStorage/chunk.hpp>
#include <CoreUtils/singletons.hpp>

namespace isis
{
namespace viewer
{


class QGLWidgetImplementation : public QGLWidget
{
	Q_OBJECT
public:
	enum PlaneType { axial, sagittal, coronal };

	QGLWidgetImplementation( ViewerCore *core, QWidget *parent = 0, PlaneType plane = axial );
	virtual ~QGLWidgetImplementation() {};
	void initializeGL();
	void setPlaneType( PlaneType plane ) { m_PlaneType = plane;}


private:
	boost::shared_ptr<ViewerCore> m_ViewerCore;
	GLuint m_CurrentTextureID;

public Q_SLOTS:


protected:
	virtual void mouseMoveEvent( QMouseEvent *e );




protected:
Q_SIGNALS:
	void redraw();

private:
	void connectSignals();



	std::vector<GLuint> m_TextureIDVec;
	PlaneType m_PlaneType;


};



}
}//end namespace

#endif