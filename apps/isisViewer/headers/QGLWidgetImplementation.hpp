#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#define GL_GLEXT_PROTOTYPES

#include <QtOpenGL/QGLWidget>
#include "ViewerCore.hpp"
#include "GLTextureHandler.hpp"
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
	explicit QGLWidgetImplementation( ViewerCore *core, QWidget *parent = 0, const QGLWidget *share = 0, OrientationHandler::PlaneOrientation orienation = OrientationHandler::axial );
	virtual ~QGLWidgetImplementation() {};
	virtual bool paintVolume( size_t imageID, size_t timestep, size_t slice );
	
private:
	ViewerCore* m_ViewerCore;
	GLuint m_CurrentTextureID;

public Q_SLOTS:


protected:
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);



protected:
Q_SIGNALS:
	void redraw();

private:
	void connectSignals();



	std::vector<GLuint> m_TextureIDVec;
	OrientationHandler::PlaneOrientation m_PlaneOrientation;


};



}
}//end namespace

#endif