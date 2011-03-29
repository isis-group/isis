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
	//look if we are inside the current viewport
	if( isInViewPort( _x, _y ) ) {
		std::pair<float, float> normCoords = widget2ViewPortCoordinates( _x, _y );
		m_CrossHair.draw( normCoords.first, normCoords.second );
	}
	redraw();
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
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	m_CrosshairCoordinates.first = width() / 2;
	m_CrosshairCoordinates.second = height() / 2;
}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();

	//TODO
	m_CurrentViewPort =
		GLOrientationHandler::calculateViewPortCoords( width(), height() );
	glViewport( m_CurrentViewPort.x, m_CurrentViewPort.y, m_CurrentViewPort.w, m_CurrentViewPort.h );
	redrawCrosshair( m_CrosshairCoordinates.first, m_CrosshairCoordinates.second );
}

void QGLWidgetImplementation::lookAtVoxel( size_t _x, size_t _y, size_t _z )
{
	
	
}


void QGLWidgetImplementation::internPaintSlice( GLuint textureID, const float *textureMatrix, float slice )
{
	redraw();
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
// 	redraw();
}

void QGLWidgetImplementation::mouseMoveEvent( QMouseEvent *e )
{
	//TODO debug
// 	lookAtVoxel( e->x(), e->y(), 50 );
}





}
} // end namespace