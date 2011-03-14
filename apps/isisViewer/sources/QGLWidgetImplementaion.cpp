#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis
{
namespace viewer
{


QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore *core, QWidget *parent, QGLWidget *share, OrientationHandler::PlaneOrientation orientation )
	: QGLWidget( parent, share ),
	  m_ViewerCore( core ),
	  m_PlaneOrientation( orientation )
{
	( new QVBoxLayout( parent ) )->addWidget( this );
	commonInit();
}

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore *core, QWidget *parent, OrientationHandler::PlaneOrientation orientation )
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

QGLWidgetImplementation* QGLWidgetImplementation::createSharedWidget( QWidget *parent, OrientationHandler::PlaneOrientation orientation )
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
	//TODO specifiy the correct slice
	paintVolume(0,0,85, false);
}

void QGLWidgetImplementation::resizeGL(int w, int h)
{
	LOG(Debug, verbose_info) << "resizeGL " << objectName().toStdString();

}


bool QGLWidgetImplementation::paintVolume( size_t imageID, size_t timestep, size_t slice, bool redrawFlag )
{

	//check if the image is available at all
	if(!m_ViewerCore->getDataContainer().isImage(imageID,timestep, slice))
	{
		LOG( Runtime, error ) << "Tried to paint image with id " << imageID << " and timestep " << timestep << 
			", but no such image exists!";
		return false;
	}
	
	//copy the volume to openGL. If this already has happend GLTextureHandler does nothing.
	GLuint textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), imageID, timestep );
	ImageHolder image = m_ViewerCore->getDataContainer()[imageID];
	OrientationHandler::VertexMatrix mat = OrientationHandler::getVertexMatrix( image, slice, m_PlaneOrientation );
	glEnable( GL_TEXTURE_3D );
	glBindTexture( GL_TEXTURE_3D, textureID );
	glBegin( GL_QUADS );
	glTexCoord3f( mat[0][0], mat[0][1], mat[0][2] );
	glVertex2f( -1.0, -1.0 );
	glTexCoord3f( mat[1][0], mat[1][1], mat[1][2] );
	glVertex2f( -1.0, 1.0 );
	glTexCoord3f( mat[2][0], mat[2][1], mat[2][2] );
	glVertex2f( 1.0, 1.0 );
	glTexCoord3f( mat[3][0], mat[3][1], mat[3][2] );
	glVertex2f( 1.0, -1.0 );		
	glEnd();
	glFlush();
	glDisable( GL_TEXTURE_3D );
	if(redrawFlag) {
		redraw();
	}
}


void QGLWidgetImplementation::mouseMoveEvent( QMouseEvent *e )
{
	//TODO debug
// 	float pos = float( e->x() ) / geometry().width();
	paintVolume(0,0,e->x());

}





}
} // end namespace