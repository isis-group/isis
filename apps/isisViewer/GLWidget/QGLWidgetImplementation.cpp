#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis
{
namespace viewer
{


QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore *core, QWidget *parent, QGLWidget *share, GLOrientationHandler::PlaneOrientation orientation )
	: QGLWidget( parent, share ),
	  m_ViewerCore( core ),
	  m_PlaneOrientation( orientation ),
	  m_ShareWidget( share )
{
	( new QVBoxLayout( parent ) )->addWidget( this );
	commonInit();
}

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore *core, QWidget *parent, GLOrientationHandler::PlaneOrientation orientation )
	: QGLWidget( parent ),
	  m_ViewerCore( core ),
	  m_PlaneOrientation( orientation )
{
	( new QVBoxLayout( parent ) )->addWidget( this );
	commonInit();
}


void QGLWidgetImplementation::commonInit()
{
	setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) );
	setMouseTracking( true );
	connectSignals();
	m_Zoom.zoomFactor = 1;
	m_Zoom.zoom = 1;
	//flags
	buttonPressed = false;



}

bool QGLWidgetImplementation::isInViewPort( size_t _x, size_t _y ) const
{
	if( _x < m_CurrentViewPort.w + m_CurrentViewPort.x
		&& _x >= m_CurrentViewPort.x
		&& _y < m_CurrentViewPort.h  + m_CurrentViewPort.y
		&& _y >= m_CurrentViewPort.y )
		{   return true; }
	else { return false; }
}

std::pair<float, float> QGLWidgetImplementation::widget2ViewPortCoordinates( size_t _x, size_t _y ) const
{
	if( isInViewPort( _x, _y ) ) {
		size_t viewPortX = _x - m_CurrentViewPort.x;
		size_t viewPortY = _y - m_CurrentViewPort.y;
		float normX = -1 + 1.0 / m_CurrentViewPort.w * viewPortX * 2;
		float normY = -1 + 1.0 / m_CurrentViewPort.h * viewPortY * 2;
		return std::make_pair<float, float>( normX, normY );
	}

}

void QGLWidgetImplementation::redrawCrosshair( size_t _x, size_t _y )
{
	size_t y = height() - _y; // make the origin the left bottom corner

	//look if we are inside the current viewport
	if( isInViewPort( _x, y ) ) {
		std::pair<float, float> normCoords = widget2ViewPortCoordinates( _x, y );
		m_CrossHair.draw( normCoords.first, normCoords.second );
		redraw();
	}


}

QGLWidgetImplementation *QGLWidgetImplementation::createSharedWidget( QWidget *parent, GLOrientationHandler::PlaneOrientation orientation )
{
	return new QGLWidgetImplementation( m_ViewerCore, parent, this, orientation );
}


void QGLWidgetImplementation::connectSignals()
{
	connect( this, SIGNAL( redraw() ), SLOT( updateGL() ) );
}

void QGLWidgetImplementation::initializeGL()
{

	LOG( Debug, verbose_info ) << "initializeGL " << objectName().toStdString();
	util::Singletons::get<GLTextureHandler, 10>().copyAllImagesToTextures( m_ViewerCore->getDataContainer() );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glGetDoublev(GL_MODELVIEW_MATRIX, m_CurrentModelView);
// 	glMatrixMode( GL_PROJECTION );
// 	glLoadIdentity();
}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();
	m_CurrentViewPort =
		GLOrientationHandler::calculateViewPort( width(), height() );
	glViewport( m_CurrentViewPort.x, m_CurrentViewPort.y, m_CurrentViewPort.w, m_CurrentViewPort.h );
	lookAtVoxel( m_CurrentVoxelCoords );
}

void QGLWidgetImplementation::lookAtVoxel( util::ivector4 coords )
{
// 	LOG(Debug, verbose_info) << objectName().toStdString() << " -> lookAtVoxel( " << coords << " ).";
	m_CurrentVoxelCoords = coords;
	//  //now we have to calculate the respective opgenGL coord for each transformedCoord
	//  //first we do this for the crosshair

	GLOrientationHandler::GLCoordinates glCoords =
		GLOrientationHandler::transformImageCoords2GLCoords( coords , m_ViewerCore->getCurrentImage(), m_CurrentViewPort, m_PlaneOrientation );
	GLOrientationHandler::MatrixType orientationMatrix = GLOrientationHandler::getOrientationMatrix( m_ViewerCore->getCurrentImage(), m_PlaneOrientation );
	float textureMatrix[16];
	GLOrientationHandler::boostMatrix2Pointer( GLOrientationHandler::orientation2TextureMatrix( orientationMatrix ), textureMatrix );

	//finally copy the image to texture memory...if necessary
	GLuint textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), 0, m_ViewerCore->getCurrentTimestep() );
	
	internPaintSlice( textureID, textureMatrix, glCoords.slice );
// 	GLdouble position[3];
// 	world2Object(glCoords.x, glCoords.y, position);
// 	std::cout << position[0] << " : " << position[1] << std::endl;
	redrawCrosshair(glCoords.x, glCoords.y );
	m_CurrentSlice = glCoords.slice;


}


void QGLWidgetImplementation::internPaintSlice( GLuint textureID, const float *textureMatrix, float slice )
{
	redraw();
	glMatrixMode(GL_MODELVIEW);
	glScalef(m_Zoom.zoomFactor, m_Zoom.zoomFactor, 1);
	glGetDoublev(GL_MODELVIEW_MATRIX, m_CurrentModelView);
	m_Zoom.zoom *= m_Zoom.zoomFactor;
	m_Zoom.zoomFactor = 1;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_TEXTURE );
	glLoadMatrixf( textureMatrix );
	glEnable( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, textureID );
	glBegin( GL_QUADS );
	glTexCoord3f( 0, 0, slice );
	glVertex2f( -1.0, -1.0 );
	glTexCoord3f( 0, 1, slice );
	glVertex2f( -1.0, 1.0 );
	glTexCoord3f( 1, 1, slice );
	glVertex2f( 1.0, 1.0 );
	glTexCoord3f( 1, 0, slice );
	glVertex2f( 1.0, -1.0 );
	glEnd();
	glFlush();
	glDisable( GL_TEXTURE_3D );
	//  redraw();
}

void QGLWidgetImplementation::mouseMoveEvent( QMouseEvent *e )
{
	if ( buttonPressed ) {
		emitMousePressEvent( e );
	}
}

void QGLWidgetImplementation::mousePressEvent( QMouseEvent *e )
{
	buttonPressed = true;
	emitMousePressEvent( e );

}

void QGLWidgetImplementation::emitMousePressEvent( QMouseEvent *e )
{
	if(isInViewPort(e->x(), height() - e->y())) {
		GLOrientationHandler::GLCoordinates glCoords;
		
		glCoords.slice = m_CurrentSlice;
		GLdouble worldCoords[3];
		object2World(e->x(), (height() - e->y()), worldCoords);
		glCoords.x = worldCoords[0];
		glCoords.y = worldCoords[1];
// 		std::cout << glCoords.x << " : " << glCoords.y << std::endl;
		util::ivector4 imageCoords = GLOrientationHandler::transformGLCoords2ImageCoords( glCoords, m_ViewerCore->getCurrentImage(), m_CurrentViewPort, m_PlaneOrientation );
// 		std::cout << imageCoords << std::endl;
		Q_EMIT voxelCoordChanged( util::ivector4( imageCoords[0], imageCoords[1], imageCoords[2], m_ViewerCore->getCurrentTimestep() ) );
	}
}

void QGLWidgetImplementation::timestepChanged( int timestep )
{
	if( m_ViewerCore->setCurrentTimestep( timestep ) ) {
		LOG( Debug, verbose_info ) << "Timestep changed to " << timestep << ".";
		lookAtVoxel( m_CurrentVoxelCoords );
	}
}

void QGLWidgetImplementation::wheelEvent(QWheelEvent* e)
{
	if(e->delta() < 0) m_Zoom.zoomFactor = 0.5;
	if(e->delta() > 0) m_Zoom.zoomFactor = 2;
	lookAtVoxel(m_CurrentVoxelCoords);
	
}


void QGLWidgetImplementation::mouseReleaseEvent( QMouseEvent *e )
{
	buttonPressed = false;
}

void QGLWidgetImplementation::object2World(int x,int y, GLdouble *world)
{
	GLdouble projectView[16];
	GLint viewport[4];
	glGetDoublev(GL_PROJECTION_MATRIX, projectView);
	glGetIntegerv(GL_VIEWPORT, viewport);
// 	for (size_t i = 0 ;i<16; i++) std::cout << m_CurrentModelView[i] << " ";
// 	std::cout << std::endl;
	gluUnProject(x,y, 0,m_CurrentModelView, projectView, viewport, &world[0], &world[1], &world[2]);
	world[0] = (world[0] + 1) / 2 * m_CurrentViewPort.w + m_CurrentViewPort.x;
	world[1] = (world[1] + 1) / 2 * m_CurrentViewPort.h + m_CurrentViewPort.y;
}

void QGLWidgetImplementation::world2Object(float x, float y, GLdouble *object)
{
	GLdouble projectView[16];
	GLint viewport[4];
	glGetDoublev(GL_PROJECTION_MATRIX, projectView);
	glGetIntegerv(GL_VIEWPORT, viewport);
	gluProject(x,y, 0,m_CurrentModelView, projectView, viewport, &object[0], &object[1], &object[2]);
	object[0] = object[0] / m_CurrentViewPort.w * 2;
	object[1] = object[1] / m_CurrentViewPort.h * 2;

}



}
} // end namespace