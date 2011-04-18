#ifndef QGLWIDGETIMPLEMENTATION_HPP
#define QGLWIDGETIMPLEMENTATION_HPP

#define GL_GLEXT_PROTOTYPES

#include <QtOpenGL/QGLWidget>
#include "QViewerCore.hpp"
#include "GLTextureHandler.hpp"
#include <iostream>
#include <DataStorage/chunk.hpp>
#include <CoreUtils/singletons.hpp>
#include "GLOrientationHandler.hpp"
#include "GLShaderHandler.hpp"
#include "GLLookupTable.hpp"

namespace isis
{
namespace viewer
{

class QViewerCore;

class QGLWidgetImplementation : public QGLWidget
{
	Q_OBJECT
public:
	enum ScalingType { no_scaling, automatic_scaling, manual_scaling };
	QGLWidgetImplementation( QViewerCore *core, QWidget *parent = 0, QGLWidget *share = 0, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );
	QGLWidgetImplementation( QViewerCore *core, QWidget *parent = 0, GLOrientationHandler::PlaneOrientation orientation = GLOrientationHandler::axial );

	QGLWidgetImplementation *createSharedWidget( QWidget *parent, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );

private:
	QGLWidgetImplementation( QViewerCore *core, QWidget *parent, QGLWidget *share, QGLContext *context, GLOrientationHandler::PlaneOrientation orienation = GLOrientationHandler::axial );
	QViewerCore *m_ViewerCore;
	QGLWidget *m_ShareWidget;

public Q_SLOTS:
	/**
	 * The function lookAtVoxel is responsible for drawing an image.
	 * If the voxelCoords are outside the image it returns false.
	 * Otherwise it will return true.
	 * @param image The image that has to be drawn
	 * @param voxelCoords The voxelCoords that has to be drawn. This also specifies the crosshair position
	 * @return True if imageID exists and voxelCoords are inside the image. Otherwise returns false.
	 */
	virtual bool lookAtVoxel( const ImageHolder &image, const util::ivector4 &voxelCoords );
	virtual bool lookAtVoxel( const util::ivector4 &voxelCoords );
	virtual bool timestepChanged( unsigned int timestep );
	virtual void setScalingType( ScalingType scalingType ) { m_ScalingType = scalingType; }
	virtual void setShowLabels( const bool show );
	virtual void setInterpolationType( const GLTextureHandler::InterpolationType interpolation );
	
protected:
	virtual void mouseMoveEvent( QMouseEvent *e );
	virtual void wheelEvent( QWheelEvent *e );
	virtual void mousePressEvent( QMouseEvent *e );
	virtual void mouseReleaseEvent( QMouseEvent *e );
	virtual void keyPressEvent( QKeyEvent *e);
	virtual void initializeGL();
	virtual void resizeGL( int w, int h );

	virtual void paintScene();
	virtual void updateStateValues( const ImageHolder &image, const util::ivector4 &voxelCoords );


protected:
Q_SIGNALS:
	void redraw();
	void voxelCoordChanged( util::ivector4 );


private:
	void connectSignals();
	void commonInit();
	void emitMousePressEvent( QMouseEvent *e );
	bool isInViewport( size_t wx, size_t wy );
	void viewLabels() ;

	std::pair<GLdouble, GLdouble> window2ObjectCoords( int16_t winx, int16_t winy, const ImageHolder &image ) const;
	std::pair<int16_t, int16_t> object2WindowCoords( GLdouble objx, GLdouble objy, const ImageHolder &image ) const;

	bool calculateTranslation( const ImageHolder &image );

	GLShaderHandler m_ScalingShader;
	GLShaderHandler m_LUTShader;
	
	std::vector<GLuint> m_TextureIDVec;
	GLOrientationHandler::PlaneOrientation m_PlaneOrientation;
	GLTextureHandler::InterpolationType m_InterplationType;

	struct State {
		State() {
			GLOrientationHandler::makeIdentity( modelViewMatrix );
			GLOrientationHandler::makeIdentity( projectionMatrix );
			GLOrientationHandler::makeIdentity( textureMatrix );
			set = false;
			opacity = 1.0;
		}
		bool set;
		GLdouble modelViewMatrix[16];
		GLdouble textureMatrix[16];
		GLdouble projectionMatrix[16];
		GLint viewport[4];
		float normalizedSlice;
		GLuint textureID;
		util::ivector4 voxelCoords;
		util::ivector4 mappedVoxelCoords;
		util::fvector4 mappedVoxelSize;
		util::ivector4 mappedImageSize;
		float opacity;
		std::pair<int16_t, int16_t> crosshairCoords;
		GLOrientationHandler::MatrixType planeOrientation;
	};

	typedef std::map<ImageHolder, State> StateMap;
	StateMap m_StateValues;

	struct Zoom {
		Zoom() {
			zoomFactorIn = 2.0;
			zoomFactorOut = 0.5;
			currentZoom = 1.0;
			zoomBorder = 0.1;
		}
		float currentZoom;
		float zoomFactorIn;
		float zoomFactorOut;
		float zoomBorder;
	} m_Zoom;

	ScalingType m_ScalingType;
	std::pair<double, double> m_ScalingPair;

	isis::viewer::GLLookUpTable m_LookUpTable;
	
	//flags
	bool zoomEventHappened;
	bool leftButtonPressed;
	bool rightButtonPressed;
	bool m_ShowLabels;

};



}
}//end namespace

#endif