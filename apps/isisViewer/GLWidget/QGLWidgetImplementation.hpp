#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#define GL_GLEXT_PROTOTYPES

#include <QtOpenGL/QGLWidget>
#include "QViewerCore.hpp"
#include "GLCrossHair.hpp"
#include "GLTextureHandler.hpp"
#include <iostream>
#include <DataStorage/chunk.hpp>
#include <CoreUtils/singletons.hpp>
#include "GLOrientationHandler.hpp"

namespace isis
{
namespace viewer
{

class QViewerCore;

class QGLWidgetImplementation : public QGLWidget
{
	Q_OBJECT
public:
	QGLWidgetImplementation( QViewerCore *core, QWidget *parent = 0, QGLWidget *share = 0, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );
	QGLWidgetImplementation( QViewerCore *core, QWidget *parent = 0, GLOrientationHandler::PlaneOrientation orientation = GLOrientationHandler::axial );



	QGLWidgetImplementation *createSharedWidget( QWidget *parent, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );

private:
	QGLWidgetImplementation( QViewerCore *core, QWidget *parent, QGLWidget *share, QGLContext *context, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );
	QViewerCore *m_ViewerCore;
	QGLWidget *m_ShareWidget;

public Q_SLOTS:
	/**
	 * The function lookAtVoxel is responsible for drawing an image with the given imageID.
	 * If the imageID does not exist or the voxelCoords are outside the image it returns false.
	 * Otherwise it will return true.
	 * @param imageID The image that has to be drawn
	 * @param voxelCoords The voxelCoords that has to be drawn. This also specifies the crosshair position
	 * @return True if imageID exists and voxelCoords are inside the image. Otherwise returns false.
	 */
	virtual bool lookAtVoxel( const ImageHolder &image, const util::ivector4 &voxelCoords);
	virtual bool lookAtVoxel( const util::ivector4 &voxelCoords);
	virtual bool timestepChanged( unsigned int timestep );

protected:
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void wheelEvent( QWheelEvent *e );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseReleaseEvent( QMouseEvent *e );
	virtual void initializeGL();
	virtual void resizeGL( int w, int h );

	virtual void paintScene();	
	virtual void updateStateValues( const ImageHolder &image, const unsigned short timestep );
	

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
	
	struct State {
		GLdouble modelViewMatrix[16];
		GLdouble textureMatrix[16];
		GLdouble projectionMatrix[16];
		GLint viewport[4];
		float normalizedSlice;
		GLuint textureID;
		util::ivector4 voxelCoords;
	};
	typedef std::map<ImageHolder, State> StateMap;
	StateMap m_StateValues;

	//flags
	bool buttonPressed;
	
};



}
}//end namespace

#endif