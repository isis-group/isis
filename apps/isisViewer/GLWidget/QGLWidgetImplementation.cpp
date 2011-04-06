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
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();
	lookAtVoxel( util::ivector4( 90, 109, 91, 0 ) );

}

void QGLWidgetImplementation::updateStateValues( const ImageHolder &image, const util::ivector4 &voxelCoords )
{
	LOG( Debug, verbose_info ) << "Updating state values for widget " << objectName().toStdString();
	m_StateValues[image].voxelCoords = voxelCoords;
	//if not happend already copy the image to GLtexture memory and return the texture id
	m_StateValues[image].textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), image, voxelCoords[3] );
	//update the texture matrix.
	//The texture matrix holds the orientation of the image and the orientation of the current widget. It does NOT hold the scaling of the image.
	GLOrientationHandler::MatrixType planeOrientatioMatrix =
		GLOrientationHandler::transformToPlaneView( image.getNormalizedImageOrientation(), m_PlaneOrientation );
	GLOrientationHandler::addOffset( planeOrientatioMatrix );
	GLOrientationHandler::boostMatrix2Pointer( planeOrientatioMatrix, m_StateValues[image].textureMatrix );

	//to visualize with the correct scaling we take the viewport
	GLOrientationHandler::recalculateViewport( width(), height(), image, planeOrientatioMatrix, m_StateValues[image].viewport );

	util::fvector4 objectCoords = GLOrientationHandler::transformVoxel2ObjectCoords( m_StateValues[image].voxelCoords, image, m_PlaneOrientation );
	m_StateValues[image].crosshairCoords = object2WindowCoords( objectCoords[0], objectCoords[1] );
	//we have to look if the crosshair is inside the window. if not, we have to translate to make it visible
	calculateTranslation( image );
	
	m_StateValues[image].normalizedSlice = objectCoords[2];
}

std::pair<GLdouble, GLdouble> QGLWidgetImplementation::window2ObjectCoords( int16_t winx, int16_t winy ) const
{
	State stateValue = m_StateValues.at( m_ViewerCore->getCurrentImage() );
	GLdouble pos[3];
	gluUnProject( winx, winy, 0, stateValue.modelViewMatrix, stateValue.projectionMatrix, stateValue.viewport , &pos[0], &pos[1], &pos[2] );
	return std::make_pair<GLdouble, GLdouble>( pos[0], pos[1] );

}

std::pair<int16_t, int16_t> QGLWidgetImplementation::object2WindowCoords( GLdouble objx, GLdouble objy ) const
{
	State stateValue = m_StateValues.at( m_ViewerCore->getCurrentImage() );
	GLdouble win[3];

	gluProject( objx, objy, 0, stateValue.modelViewMatrix, stateValue.projectionMatrix, stateValue.viewport, &win[0], &win[1], &win[2] );
	return std::make_pair<int16_t, int16_t>( win[0] - stateValue.viewport[0], win[1] - stateValue.viewport[1] );
}

bool QGLWidgetImplementation::calculateTranslation( const ImageHolder &image )
{
	std::pair<int16_t, int16_t> center = std::make_pair<int16_t, int16_t>( width() / 2, height() / 2);
	if(center.first != m_StateValues[image].crosshairCoords.first) {
		m_StateValues[image].modelViewMatrix[12] = -(1.0 / m_StateValues[image].viewport[2] ) * (m_StateValues[image].crosshairCoords.first - center.first) ;	
		m_StateValues[image].modelViewMatrix[13] = -(1.0 / m_StateValues[image].viewport[3] ) * (m_StateValues[image].crosshairCoords.second - center.second) ;
	}
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixd( m_StateValues[image].modelViewMatrix );

}

bool QGLWidgetImplementation::lookAtVoxel( const isis::util::ivector4 &voxelCoords )
{
	//someone has told the widget to paint a list of images with the respective ids.
	//So first we have to update the state values for each image
	BOOST_FOREACH( DataContainer::const_reference image, m_ViewerCore->getDataContainer() ) {
		updateStateValues( image, voxelCoords );
	}
	paintScene();

}


bool QGLWidgetImplementation::lookAtVoxel( const ImageHolder &image, const util::ivector4 &voxelCoords )
{
	updateStateValues( image, voxelCoords );
	paintScene();

}


void QGLWidgetImplementation::paintScene()
{
	redraw();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	BOOST_FOREACH( StateMap::const_reference currentImage, m_StateValues ) {
		glViewport( currentImage.second.viewport[0], currentImage.second.viewport[1], currentImage.second.viewport[2], currentImage.second.viewport[3] );
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixd( currentImage.second.projectionMatrix );
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixd( currentImage.second.modelViewMatrix );
		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();
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
	//paint crosshair
	State currentState = m_StateValues.at( m_ViewerCore->getCurrentImage() );
	glColor4f( 1, 0, 0, 0 );
	glLineWidth( 1.0 );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0, currentState.viewport[2], 0, currentState.viewport[3], -1, 1 );
	glBegin( GL_LINES );
	unsigned short gap = height() / 20;
	glVertex3i( currentState.crosshairCoords.first , 0, 1 );
	glVertex3i( currentState.crosshairCoords.first, currentState.crosshairCoords.second - gap , 1 );
	glVertex3i( currentState.crosshairCoords.first, currentState.crosshairCoords.second + gap, 1 );
	glVertex3i( currentState.crosshairCoords.first, height(), 1 );

	glVertex3i( 0, currentState.crosshairCoords.second, 1 );
	glVertex3i( currentState.crosshairCoords.first - gap, currentState.crosshairCoords.second, 1 );
	glVertex3i( currentState.crosshairCoords.first + gap, currentState.crosshairCoords.second, 1 );
	glVertex3i( width(), currentState.crosshairCoords.second, 1 );
	glEnd();
	glPointSize( 2.0 );
	glBegin( GL_POINTS );
	glVertex3d( currentState.crosshairCoords.first, currentState.crosshairCoords.second, 1 );
	glEnd();
	glFlush();
	glLoadIdentity();

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
	std::pair<float, float> objectCoords = window2ObjectCoords( e->x(), ( height() - e->y() ) );
	util::ivector4 voxelCoords = GLOrientationHandler::transformObject2VoxelCoords( util::fvector4( objectCoords.first, objectCoords.second, m_StateValues.at( m_ViewerCore->getCurrentImage() ).normalizedSlice ), m_ViewerCore->getCurrentImage(), m_PlaneOrientation );

	Q_EMIT voxelCoordChanged( voxelCoords );
}

bool QGLWidgetImplementation::timestepChanged( unsigned int timestep )
{
	BOOST_FOREACH( StateMap::reference currentImage, m_StateValues ) {
		if( currentImage.first.getImageSize()[3] > timestep ) {
			currentImage.second.voxelCoords[3] = timestep;
		} else {
			currentImage.second.voxelCoords[3] = currentImage.first.getImageSize()[3] - 1;
		}

		lookAtVoxel( currentImage.first, currentImage.second.voxelCoords );
	}

}

void QGLWidgetImplementation::wheelEvent( QWheelEvent *e )
{
	float zoomFactor = 1;
	if(e->delta() < 0 )
	{
		zoomFactor = m_Zoom.zoomFactorOut;
	} else if (e->delta() > 0 ) { zoomFactor = m_Zoom.zoomFactorIn; } 
	m_Zoom.currentZoom *= zoomFactor;
	if(m_Zoom.currentZoom >= 1) {
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(m_StateValues[m_ViewerCore->getCurrentImage()].projectionMatrix );
		glScalef(zoomFactor, zoomFactor, 1);
		glGetDoublev( GL_PROJECTION_MATRIX, m_StateValues[m_ViewerCore->getCurrentImage()].projectionMatrix );
		glLoadIdentity();
		lookAtVoxel(m_StateValues[m_ViewerCore->getCurrentImage()].voxelCoords);
	} else {
		m_Zoom.currentZoom = 1;
	}

}




void QGLWidgetImplementation::mouseReleaseEvent( QMouseEvent *e )
{
	buttonPressed = false;
}



}
} // end namespace