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

	void redrawCrosshair( size_t x, size_t y );

	


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
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseReleaseEvent( QMouseEvent *e );
	virtual void initializeGL();
	virtual void resizeGL( int w, int h );
	///this function modifies the crosshair and the slice position to the given image voxel coords
	


protected:
Q_SIGNALS:
	void redraw();
	void voxelCoordChanged( util::ivector4 );

private:
	void connectSignals();
	void commonInit();
	void internPaintSlice( GLuint textureID, const float *matrix, float normalizedSlice );
	bool isInViewPort( size_t x, size_t y ) const;
	std::pair<float, float> widget2ViewPortCoordinates( size_t x, size_t y ) const;
	void emitMousePressEvent( QMouseEvent *e );

	std::vector<GLuint> m_TextureIDVec;
	GLOrientationHandler::PlaneOrientation m_PlaneOrientation;
	util::ivector4 m_CurrentVoxelCoords;

	GLOrientationHandler::ViewPortCoords m_CurrentViewPort;
	GLCrossHair m_CrossHair;
	GLuint m_CurrentTextureID;
	float m_CurrentSlice;
	
	//flags
	bool buttonPressed;

};



}
}//end namespace

#endif