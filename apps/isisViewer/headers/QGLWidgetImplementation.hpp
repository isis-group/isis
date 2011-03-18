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
	QGLWidgetImplementation( ViewerCore *core, QWidget *parent = 0, QGLWidget *share = 0, OrientationHandler::PlaneOrientation orienation = OrientationHandler::axial );
	QGLWidgetImplementation( ViewerCore *core, QWidget *parent = 0, OrientationHandler::PlaneOrientation orientation = OrientationHandler::axial );

	virtual bool paintSlice( size_t imageID, size_t timestep, size_t slice );
	void redrawCrosshair( size_t x, size_t y );

	virtual void lookAtVoxel( size_t x, size_t y, size_t z );


	QGLWidgetImplementation *createSharedWidget( QWidget *parent, OrientationHandler::PlaneOrientation orienation = OrientationHandler::axial );

private:
	QGLWidgetImplementation( ViewerCore *core, QWidget *parent, QGLWidget *share, QGLContext *context, OrientationHandler::PlaneOrientation orienation = OrientationHandler::axial );
	ViewerCore *m_ViewerCore;
	QGLWidget *m_ShareWidget;

public Q_SLOTS:


protected:
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void initializeGL();
	virtual void resizeGL( int w, int h );
	virtual void paintGL();


protected:
Q_SIGNALS:
	void redraw();

private:
	void connectSignals();
	void commonInit();
	void internPaintSlice( GLuint textureID, const float *matrix, float slice );
	bool isInViewPort( size_t x, size_t y ) const;
	std::pair<float, float> widget2ViewPortCoordinates( size_t x, size_t y ) const;

	std::vector<GLuint> m_TextureIDVec;
	OrientationHandler::PlaneOrientation m_PlaneOrientation;
	std::pair<size_t, size_t> m_CrosshairCoordinates;

	OrientationHandler::ViewPortCoords m_CurrentViewPort;
	GLCrossHair m_CrossHair;
	GLuint m_CurrentTextureID;
	float m_CurrentSlice;

};



}
}//end namespace

#endif