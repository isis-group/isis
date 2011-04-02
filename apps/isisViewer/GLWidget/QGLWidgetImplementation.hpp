#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#define GL_GLEXT_PROTOTYPES

#include <QtOpenGL/QGLWidget>
#include "ViewerCore.hpp"
#include "GLCrossHair.hpp"
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
	QGLWidgetImplementation( ViewerCore *core, QWidget *parent = 0, QGLWidget *share = 0, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );
	QGLWidgetImplementation( ViewerCore *core, QWidget *parent = 0, GLOrientationHandler::PlaneOrientation orientation = GLOrientationHandler::axial );



	QGLWidgetImplementation *createSharedWidget( QWidget *parent, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );

private:
	QGLWidgetImplementation( ViewerCore *core, QWidget *parent, QGLWidget *share, QGLContext *context, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );
	ViewerCore *m_ViewerCore;
	QGLWidget *m_ShareWidget;

public Q_SLOTS:
	virtual void lookAtVoxel( util::ivector4 );
	virtual void timestepChanged( int );

protected:
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void wheelEvent( QWheelEvent *e );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseReleaseEvent( QMouseEvent *e );
	virtual void initializeGL();
	virtual void resizeGL( int w, int h );

	virtual void paintScene();	


protected:
Q_SIGNALS:
	void redraw();
	void voxelCoordChanged( util::ivector4 );

private:
	void connectSignals();
	void commonInit();
	void emitMousePressEvent( QMouseEvent *e );

	std::vector<GLuint> m_TextureIDVec;
	GLOrientationHandler::PlaneOrientation m_PlaneOrientation;
	GLCrossHair m_CrossHair;
	
	struct {
		GLdouble modelViewMatrix[16];
		GLdouble textureMatrix[16];
		GLdouble projectionMatrix[16];
		GLint viewport[4];
		float normalizedSlice;
		GLuint textureID;
		util::ivector4 voxelCoords;
	} m_StateValues;

	

	//flags
	bool buttonPressed;
	
};



}
}//end namespace

#endif