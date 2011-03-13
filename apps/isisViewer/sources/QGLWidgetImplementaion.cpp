#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis
{
namespace viewer
{

QGLWidgetImplementation::QGLWidgetImplementation( ViewerCore *core, QWidget *parent, const QGLWidget *share, PlaneType plane )
	: QGLWidget( parent, share ),
	  m_ViewerCore( boost::shared_ptr<ViewerCore>( core ) ),
	  m_PlaneType( plane )
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
	//here we take a singleton to ensure having only one instance which manages the textures for all widgets
	util::Singletons::get<GLTextureHandler, 10>().copyAllImagesToTextures( m_ViewerCore->getDataContainer() );
}


void QGLWidgetImplementation::mouseMoveEvent( QMouseEvent *e )
{
	//TODO debug
	float pos = float( e->x() ) / 300;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_TEXTURE_3D );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
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