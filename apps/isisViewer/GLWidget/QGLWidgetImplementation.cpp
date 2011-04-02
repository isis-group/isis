#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>

namespace isis
{
namespace viewer
{


QGLWidgetImplementation::QGLWidgetImplementation( QViewerCore *core, QWidget *parent, QGLWidget *share, GLOrientationHandler::PlaneOrientation orientation )
	: QGLWidget( parent, share ),
	  m_ViewerCore( core ),
	  m_PlaneOrientation( orientation ),
	  m_ShareWidget( share )
{
	( new QVBoxLayout( parent ) )->addWidget( this );
	commonInit();
}

QGLWidgetImplementation::QGLWidgetImplementation( QViewerCore *core, QWidget *parent, GLOrientationHandler::PlaneOrientation orientation )
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
}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();
	
}

void QGLWidgetImplementation::updateStateValues( const ImageHolder &image, const unsigned short timestep )
{
	//if not happend already copy the image to GLtexture memory and return the texture id
	m_StateValues[image].textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), image, timestep );	
	//update the texture matrix. 
	//The texture matrix holds the orientation of the image and the orientation of the current widget. It does NOT hold the scaling of the image.
	GLOrientationHandler::MatrixType planeOrientatioMatrix = 
		GLOrientationHandler::transformToPlaneView( image.getNormalizedImageOrientation(), m_PlaneOrientation );
	GLOrientationHandler::boostMatrix2Pointer( planeOrientatioMatrix, m_StateValues[image].textureMatrix );
	
	

}




bool QGLWidgetImplementation::lookAtVoxel( const isis::util::ivector4& voxelCoords)
{
	//someone has told the widget to paint a list of images with the respective ids. 
	//So first we have to update the state values for each image
	m_StateValues.clear();
	BOOST_FOREACH( DataContainer::const_reference image, m_ViewerCore->getDataContainer() )
	{
		updateStateValues( image, voxelCoords[3] );
	}

}


bool QGLWidgetImplementation::lookAtVoxel( const ImageHolder &image, const util::ivector4 &coords )
{
	m_StateValues[image].voxelCoords = coords;


}


void QGLWidgetImplementation::paintScene()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	BOOST_FOREACH( StateMap::const_reference currentImage, m_StateValues )
	{
		glMatrixMode( GL_TEXTURE );
		glLoadMatrixd( currentImage.second.textureMatrix );
		glEnable( GL_TEXTURE_3D );
		glBindTexture( GL_TEXTURE_3D, currentImage.second.textureID );
		glBegin( GL_QUADS );
		glTexCoord3f( 0, 0, currentImage.second.normalizedSlice );
		glVertex2f( -1.0, -1.0 );
		glTexCoord3f( 0, 1, currentImage.second.normalizedSlice );
		glVertex2f( -1.0, 1.0 );
		glTexCoord3f( 1, 1, currentImage.second.normalizedSlice );
		glVertex2f( 1.0, 1.0 );
		glTexCoord3f( 1, 0, currentImage.second.normalizedSlice );
		glVertex2f( 1.0, -1.0 );
		glEnd();
		glDisable( GL_TEXTURE_3D );
	}
	glFlush();
	redraw();
		
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
	
// 	Q_EMIT voxelCoordChanged( m_StateValues.voxelCoords );
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