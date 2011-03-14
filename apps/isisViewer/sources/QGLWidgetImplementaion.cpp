#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis
{
namespace viewer
{

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore *core, QWidget *parent, const QGLWidget *share, OrientationHandler::PlaneOrientation orientation )
	: QGLWidget( parent, share ),
	  m_ViewerCore( core ),
	  m_PlaneOrientation( orientation )
{
	setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored ) );
	( new QVBoxLayout( parent ) )->addWidget( this );
	setMouseTracking( true );
	connectSignals();

}

void QGLWidgetImplementation::connectSignals()
{
	connect( this, SIGNAL( redraw() ), SLOT( updateGL() ) );

}
void QGLWidgetImplementation::initializeGL()
{
	LOG( Debug, verbose_info ) << "initializeGL";
	//here we take a singleton to ensure having only one instance which manages the textures for all widgets
	util::Singletons::get<GLTextureHandler, 10>().copyAllImagesToTextures( m_ViewerCore->getDataContainer() );
	paintVolume(0,0,85);
}

void QGLWidgetImplementation::resizeGL(int w, int h)
{
	LOG(Debug, verbose_info) << "resizeGL";

}


bool QGLWidgetImplementation::paintVolume( size_t imageID, size_t timestep, size_t slice )
{

	//check if the image is available at all
	if(!m_ViewerCore->getDataContainer().isImage(imageID,timestep, slice))
	{
		LOG( Runtime, error ) << "Tried to paint image with id " << imageID << " and timestep " << timestep << 
			", but no such image exists!";
		return false;
	}
	ImageHolder image = m_ViewerCore->getDataContainer()[imageID];
	//copy the volume to openGL. If this already has happend GLTextureHandler does nothing.
	GLuint textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), imageID, timestep );
	float glSlice = (1.0 / OrientationHandler::getNumberOfSlices(image, m_PlaneOrientation) ) * slice;
	glEnable( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, textureID );
	glBegin( GL_QUADS );
	glTexCoord3f( glSlice, 0.0, 0.0 );
	glVertex3f( 1.0, 1.0, 0 );
	glTexCoord3f( glSlice, 0.0, 1.0 );
	glVertex3f( 1.0, -1.0, 0 );
	glTexCoord3f( glSlice, 1.0, 1.0 );
	glVertex3f( -1.0, -1.0, 0 );
	glTexCoord3f( glSlice, 1.0, 0.0 );
	glVertex3f( -1.0, 1.0, 0 );
	glEnd();
	glFlush();
	glDisable( GL_TEXTURE_3D );
}


void QGLWidgetImplementation::mouseMoveEvent( QMouseEvent *e )
{
	//TODO debug
	float pos = float( e->x() ) / geometry().width();
// 	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_TEXTURE_3D );
// 	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
	glBindTexture( GL_TEXTURE_3D, 1 );
	glBegin( GL_QUADS );
	glTexCoord3f( pos, 0.0, 0.0 );
	glVertex3f( 1.0, 1.0, 0 );
	glTexCoord3f( pos, 0.0, 1.0 );
	glVertex3f( 1.0, -1.0, 0 );
	glTexCoord3f( pos, 1.0, 1.0 );
	glVertex3f( -1.0, -1.0, 0 );
	glTexCoord3f( pos, 1.0, 0.0 );
	glVertex3f( -1.0, 1.0, 0 );
	glEnd();
	glFlush();
	glDisable( GL_TEXTURE_3D );
	redraw();

}





}
} // end namespace