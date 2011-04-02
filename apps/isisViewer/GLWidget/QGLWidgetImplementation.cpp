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
	//flags
	buttonPressed = false;



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
	glGetDoublev(GL_MODELVIEW_MATRIX, m_StateValues.modelViewMatrix);
// 	glMatrixMode( GL_PROJECTION );
// 	glLoadIdentity();
}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();
	
}

bool QGLWidgetImplementation::lookAtVoxel( const unsigned short imageID, const util::ivector4 &coords )
{
	m_StateValues.voxelCoords = coords;


}


void QGLWidgetImplementation::paintScene()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_TEXTURE );
	glLoadMatrixd( m_StateValues.textureMatrix );
	glEnable( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, m_StateValues.textureID );
	glBegin( GL_QUADS );
	glTexCoord3f( 0, 0, m_StateValues.normalizedSlice );
	glVertex2f( -1.0, -1.0 );
	glTexCoord3f( 0, 1, m_StateValues.normalizedSlice );
	glVertex2f( -1.0, 1.0 );
	glTexCoord3f( 1, 1, m_StateValues.normalizedSlice );
	glVertex2f( 1.0, 1.0 );
	glTexCoord3f( 1, 0, m_StateValues.normalizedSlice );
	glVertex2f( 1.0, -1.0 );
	glEnd();
	glFlush();
	glDisable( GL_TEXTURE_3D );
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
	
	Q_EMIT voxelCoordChanged( m_StateValues.voxelCoords );
}

bool QGLWidgetImplementation::timestepChanged( unsigned int timestep )
{
	
}

void QGLWidgetImplementation::wheelEvent(QWheelEvent* e)
{
	
	
}


void QGLWidgetImplementation::mouseReleaseEvent( QMouseEvent *e )
{
	buttonPressed = false;
}



}
} // end namespace