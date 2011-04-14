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
	m_ScalingType = automatic_scaling;
	m_ScalingPair = std::make_pair<double, double>(0.0,1.0);
	//flags
	leftButtonPressed = false;
	rightButtonPressed = false;
	
	
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
	util::Singletons::get<GLTextureHandler, 10>().copyAllImagesToTextures( m_ViewerCore->getDataContainer() );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	std::string scalingShader = " uniform float extent; uniform float bias; uniform float scaling; uniform sampler3D imageTexture;  void main() { vec4 color = texture3D(imageTexture, gl_TexCoord[0].xyz); gl_FragColor = (color + bias/extent) * scaling; }";
	
	m_ScalingShader.createContext();
	m_ScalingShader.addShader( "scaling", scalingShader, GLShader::fragment );
	
}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();
	if( m_ViewerCore->getDataContainer().size() ) {
		//TODO show the center of the image
		lookAtVoxel( util::ivector4( 90, 109, 91 ) );
	}
}

void QGLWidgetImplementation::updateStateValues( const ImageHolder &image, const util::ivector4 &voxelCoords )
{
	LOG( Debug, verbose_info ) << "Updating state values for widget " << objectName().toStdString();
	State &state = m_StateValues[image];
	state.voxelCoords = voxelCoords;
	//check if we are inside the image
	for( size_t i = 0; i<4; i++) {
		state.voxelCoords[i] = state.voxelCoords[i] < 0 ? 0 : state.voxelCoords[i];
		state.voxelCoords[i] = state.voxelCoords[i] >= image.getImageSize()[i] ? image.getImageSize()[i] - 1 : state.voxelCoords[i];
	}
	//if not happend already copy the image to GLtexture memory and return the texture id
	state.textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), image, voxelCoords[3] );

	//update the texture matrix.
	//The texture matrix holds the orientation of the image and the orientation of the current widget. It does NOT hold the scaling of the image.
	if( ! state.set ) {
		state.planeOrientation = 
			GLOrientationHandler::transformToPlaneView( image.getNormalizedImageOrientation(), m_PlaneOrientation );
		GLOrientationHandler::boostMatrix2Pointer( GLOrientationHandler::addOffset( state.planeOrientation ), state.textureMatrix );
		state.mappedVoxelSize = GLOrientationHandler::transformVector<float>( image.getPropMap().getPropertyAs<util::fvector4>( "voxelSize" ) + image.getPropMap().getPropertyAs<util::fvector4>( "voxelGap" ) , state.planeOrientation );
		state.mappedImageSize = GLOrientationHandler::transformVector<int>( image.getImageSize(), state.planeOrientation );
		state.set = true;
	}
	state.mappedVoxelCoords = GLOrientationHandler::transformVector<int>( state.voxelCoords, state.planeOrientation );
	//to visualize with the correct scaling we take the viewport
	GLOrientationHandler::recalculateViewport( width(), height(), state.mappedVoxelSize, state.mappedImageSize, state.viewport );
	if( rightButtonPressed ) {
		calculateTranslation( image );
	}
	
	util::dvector4 objectCoords = GLOrientationHandler::transformVoxel2ObjectCoords( state.voxelCoords, image, state.planeOrientation );
	state.crosshairCoords = object2WindowCoords( objectCoords[0], objectCoords[1], image );
	state.normalizedSlice = objectCoords[2];

}

std::pair<GLdouble, GLdouble> QGLWidgetImplementation::window2ObjectCoords( int16_t winx, int16_t winy, const ImageHolder &image ) const
{
	const State &stateValue = m_StateValues.at(image);
	GLdouble pos[3];
	gluUnProject( winx, winy, 0, stateValue.modelViewMatrix, stateValue.projectionMatrix, stateValue.viewport , &pos[0], &pos[1], &pos[2] );
	return std::make_pair<GLdouble, GLdouble>( pos[0], pos[1] );

}

std::pair<int16_t, int16_t> QGLWidgetImplementation::object2WindowCoords( GLdouble objx, GLdouble objy, const ImageHolder &image ) const
{
	const State &stateValue = m_StateValues.at( image );
	GLdouble win[3];
	GLdouble pro[16];
	gluProject( objx, objy, 0, stateValue.modelViewMatrix, stateValue.projectionMatrix, stateValue.viewport, &win[0], &win[1], &win[2] );
	return std::make_pair<int16_t, int16_t>( (win[0] - stateValue.viewport[0]), win[1] - stateValue.viewport[1] );
}

bool QGLWidgetImplementation::calculateTranslation( const ImageHolder &image )
{
	State &state = m_StateValues[image];
	std::pair<int16_t, int16_t> center = std::make_pair<int16_t, int16_t>( abs(state.mappedImageSize[0]) / 2, abs(state.mappedImageSize[1]) / 2 );
	float shiftX = center.first - (state.mappedVoxelCoords[0] < 0 ? abs(state.mappedImageSize[0]) + state.mappedVoxelCoords[0] : state.mappedVoxelCoords[0]);
	float shiftY =  center.second - (state.mappedVoxelCoords[1] < 0 ? abs(state.mappedImageSize[1]) + state.mappedVoxelCoords[1] : state.mappedVoxelCoords[1]);
	GLdouble *mat = state.modelViewMatrix;
	state.modelViewMatrix[12] = (1.0 / abs(state.mappedImageSize[0])) * shiftX ;
	state.modelViewMatrix[13] = (1.0 / abs(state.mappedImageSize[1])) * shiftY ;
	float factor = 1 + (0.5 * (m_Zoom.currentZoom / 2 - 1) );
	mat[12] = state.modelViewMatrix[12] * factor ;
	mat[13] = state.modelViewMatrix[13] * factor;
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glLoadMatrixd( mat );

}

bool QGLWidgetImplementation::lookAtVoxel( const isis::util::ivector4 &voxelCoords )
{
	LOG( Debug, verbose_info ) << "Looking at voxel: " << voxelCoords;
	//someone has told the widget to paint all available images.
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
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	m_ScalingShader.setEnabled( true );
	BOOST_FOREACH( StateMap::const_reference currentImage, m_StateValues ) {
		double scaling, bias;
		if(m_ScalingType == automatic_scaling) {
			scaling = currentImage.first.getOptimalScalingPair().second;
			bias = currentImage.first.getOptimalScalingPair().first;
		} else if ( m_ScalingType == manual_scaling) {
			scaling = m_ScalingPair.second;
			bias = m_ScalingPair.first;
		} else {
			scaling = 1.0;
			bias = 0.0;
		}
		glViewport( currentImage.second.viewport[0], currentImage.second.viewport[1], currentImage.second.viewport[2], currentImage.second.viewport[3] );
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();
		glLoadMatrixd( currentImage.second.projectionMatrix );
		glMatrixMode( GL_MODELVIEW );
		glLoadMatrixd( currentImage.second.modelViewMatrix );
		glMatrixMode( GL_TEXTURE );
		glLoadIdentity();
		glLoadMatrixd( currentImage.second.textureMatrix );
		glEnable( GL_TEXTURE_3D );
		glBindTexture( GL_TEXTURE_3D, currentImage.second.textureID );
		m_ScalingShader.addVariable<float>("textureImage", 0);
		m_ScalingShader.addVariable<float>("extent", std::numeric_limits<GLubyte>::max());
		m_ScalingShader.addVariable<float>("scaling", scaling);
		m_ScalingShader.addVariable<float>("bias", bias);
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
	m_ScalingShader.setEnabled( false );
	//paint crosshair
	glDisable(GL_BLEND);
	const State &currentState = m_StateValues.at( m_ViewerCore->getCurrentImage() );
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

	if ( rightButtonPressed || leftButtonPressed ) {
		emitMousePressEvent( e );
	}
}

void QGLWidgetImplementation::mousePressEvent( QMouseEvent *e )
{
	if(e->button() == Qt::LeftButton ) {
		leftButtonPressed = true;
	}
	if(e->button() == Qt::RightButton ) {
		rightButtonPressed = true;
	}
	emitMousePressEvent( e );

}

bool QGLWidgetImplementation::isInViewport(size_t wx, size_t wy)
{
	GLint *viewport = m_StateValues[m_ViewerCore->getCurrentImage()].viewport;
	if((wx > viewport[0] && wx < (viewport[0] + viewport[2])) && ( wy > viewport[1] && wy < (viewport[1] + viewport[3]))) 
	{
		return true;
	} else {
		return false;
	}
}


void QGLWidgetImplementation::emitMousePressEvent( QMouseEvent *e )
{
	if( isInViewport( e->x(), e->y() ) ) {
		std::pair<float, float> objectCoords = window2ObjectCoords( e->x(), height() - e->y(), m_ViewerCore->getCurrentImage() );
		util::ivector4 voxelCoords = GLOrientationHandler::transformObject2VoxelCoords( util::fvector4( objectCoords.first, objectCoords.second, m_StateValues.at( m_ViewerCore->getCurrentImage() ).normalizedSlice ), m_ViewerCore->getCurrentImage(), m_PlaneOrientation );
		Q_EMIT voxelCoordChanged( voxelCoords );
	}
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

	if( e->delta() < 0 ) {
		zoomFactor = m_Zoom.zoomFactorOut;
	} else if ( e->delta() > 0 ) { zoomFactor = m_Zoom.zoomFactorIn; }

	m_Zoom.currentZoom *= zoomFactor;

	if( m_Zoom.currentZoom >= 1 ) {
		glMatrixMode( GL_PROJECTION );
		glLoadMatrixd( m_StateValues[m_ViewerCore->getCurrentImage()].projectionMatrix );
		glScalef( zoomFactor, zoomFactor, 1 );
		glGetDoublev( GL_PROJECTION_MATRIX, m_StateValues[m_ViewerCore->getCurrentImage()].projectionMatrix );
		glLoadIdentity();
		lookAtVoxel( m_StateValues[m_ViewerCore->getCurrentImage()].voxelCoords );
	} else {
		m_Zoom.currentZoom = 1;
	}

}

void QGLWidgetImplementation::mouseReleaseEvent( QMouseEvent *e )
{	
	if(e->button() == Qt::LeftButton ) {
		leftButtonPressed = false;
	}
	if(e->button() == Qt::RightButton ) {
		rightButtonPressed = false;
	}
}

void QGLWidgetImplementation::setMinMaxRangeChanged( std::pair<double, double> minMax)
{
	//TODO this is has to be implemented
	util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( m_ViewerCore->getDataContainer(), m_ViewerCore->getCurrentImage(),m_ViewerCore->getCurrentTimestep() );
	lookAtVoxel( m_StateValues[m_ViewerCore->getCurrentImage()].voxelCoords );
}


}
} // end namespace