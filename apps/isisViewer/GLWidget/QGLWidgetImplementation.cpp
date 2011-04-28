#include "QGLWidgetImplementation.hpp"
#include <QVBoxLayout>
#include <QMouseEvent>
#include "GLShaderCode.hpp"
#include <sys/stat.h>


namespace isis
{
namespace viewer
{


QGLWidgetImplementation::QGLWidgetImplementation( QViewerCore *core, QWidget *parent, QGLWidget *share, PlaneOrientation orientation )
	: QGLWidget( parent, share ),
	  m_ViewerCore( core ),
	  m_PlaneOrientation( orientation ),
	  m_ShareWidget( share )
{
	( new QVBoxLayout( parent ) )->addWidget( this );
	commonInit();
}

QGLWidgetImplementation::QGLWidgetImplementation( QViewerCore *core, QWidget *parent, PlaneOrientation orientation )
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
	setFocusPolicy( Qt::StrongFocus );
	setMouseTracking( true );
	connectSignals();
	m_ScalingType = no_scaling;
	m_InterplationType = GLTextureHandler::neares_neighbor;
	m_ScalingPair = std::make_pair<double, double>( 0.0, 1.0 );
	//flags
	zoomEventHappened = false;
	leftButtonPressed = false;
	rightButtonPressed = false;
	init = true;
	m_ShowLabels = false;

}



QGLWidgetImplementation *QGLWidgetImplementation::createSharedWidget( QWidget *parent, PlaneOrientation orientation )
{
	return new QGLWidgetImplementation( m_ViewerCore, parent, this, orientation );
}

void QGLWidgetImplementation::addImage( const boost::shared_ptr<ImageHolder> image )
{
	m_StateValues.insert( std::make_pair<boost::shared_ptr<ImageHolder>, State>
						  ( image  , State() ) );
}

bool QGLWidgetImplementation::removeImage( const boost::shared_ptr<ImageHolder> image )
{
}


void QGLWidgetImplementation::connectSignals()
{
	connect( this, SIGNAL( redraw() ), SLOT( updateGL() ) );

}

void QGLWidgetImplementation::initializeGL()
{
	util::Singletons::get<GLTextureHandler, 10>().copyAllImagesToTextures( m_ViewerCore->getDataContainer(), true, m_InterplationType );
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glShadeModel( GL_FLAT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	m_ScalingShader.createContext();
	m_LUTShader.createContext();
	m_ScalingShader.addShader( "scaling", scaling_shader_code, GLShader::fragment );
	m_LUTShader.addShader( "lut", colormap_shader_code, GLShader::fragment );

}


void QGLWidgetImplementation::resizeGL( int w, int h )
{
	makeCurrent();
	LOG( Debug, verbose_info ) << "resizeGL " << objectName().toStdString();

	if( m_ViewerCore->getDataContainer().size() ) {
		if( init ) {
			util::ivector4 size = m_ViewerCore->getCurrentImage()->getImageSize();
			lookAtPhysicalCoords( m_ViewerCore->getCurrentImage()->getImage()->getPhysicalCoordsFromIndex( util::ivector4( size[0] / 2, size[1] / 2, size[2] / 2 ) ) );
			init = false;
		} else {
			updateScene();
		}

	}
}

void QGLWidgetImplementation::updateStateValues( boost::shared_ptr<ImageHolder> image, const util::ivector4 &voxelCoords )
{
	LOG( Debug, verbose_info ) << "Updating state values for widget " << objectName().toStdString();
	State &state = m_StateValues.at(image);
	unsigned int timestep = state.voxelCoords[3];
	state.voxelCoords = voxelCoords;
	state.voxelCoords[3] = timestep;

	//check if we are inside the image
	for( size_t i = 0; i < 4; i++ ) {
		state.voxelCoords[i] = state.voxelCoords[i] < 0 ? 0 : state.voxelCoords[i];
		state.voxelCoords[i] = state.voxelCoords[i] >= image->getImageSize()[i] ? image->getImageSize()[i] - 1 : state.voxelCoords[i];
		
	}
	//if not happend already copy the image to GLtexture memory and return the texture id
	state.textureID = util::Singletons::get<GLTextureHandler, 10>().copyImageToTexture( image, state.voxelCoords[3], true, m_InterplationType );

	//update the texture matrix.
	//The texture matrix holds the orientation of the image and the orientation of the current widget. It does NOT hold the scaling of the image.
	if( ! state.set ) {
		state.planeOrientation =
			GLOrientationHandler::transformToPlaneView( image->getNormalizedImageOrientation(), m_PlaneOrientation );
		GLOrientationHandler::boostMatrix2Pointer( GLOrientationHandler::addOffset( state.planeOrientation ), state.textureMatrix );
		state.mappedVoxelSize = GLOrientationHandler::transformVector<float>( image->getPropMap().getPropertyAs<util::fvector4>( "voxelSize" ) + image->getPropMap().getPropertyAs<util::fvector4>( "voxelGap" ) , state.planeOrientation );
		state.mappedImageSize = GLOrientationHandler::transformVector<int>( image->getImageSize(), state.planeOrientation );
		state.set = true;
	}
	
	state.mappedVoxelCoords = GLOrientationHandler::transformVector<int>( state.voxelCoords, state.planeOrientation );
	//to visualize with the correct scaling we take the viewport
	unsigned short border = 0;

	if( m_ShowLabels ) {
		border = 30;
	}

	GLOrientationHandler::recalculateViewport( width(), height(), state.mappedVoxelSize, state.mappedImageSize, state.viewport, border );
	if( rightButtonPressed || zoomEventHappened && image.get() == m_ViewerCore->getCurrentImage().get() ) {
		zoomEventHappened = false;
		calculateTranslation( );
	}
	

	util::dvector4 objectCoords = GLOrientationHandler::transformVoxel2ObjectCoords( state.voxelCoords, image, state.planeOrientation );
	state.crosshairCoords = object2WindowCoords( objectCoords[0], objectCoords[1], image );
	state.normalizedSlice = objectCoords[2];

}

std::pair<GLdouble, GLdouble> QGLWidgetImplementation::window2ObjectCoords( int16_t winx, int16_t winy, const boost::shared_ptr<ImageHolder> image ) const
{
	const State &stateValue = m_StateValues.at( image );
	GLdouble pos[3];
	gluUnProject( winx, winy, 0, stateValue.modelViewMatrix, stateValue.projectionMatrix, stateValue.viewport , &pos[0], &pos[1], &pos[2] );
	return std::make_pair<GLdouble, GLdouble>( pos[0], pos[1] );

}

std::pair<int16_t, int16_t> QGLWidgetImplementation::object2WindowCoords( GLdouble objx, GLdouble objy, const boost::shared_ptr<ImageHolder> image ) const
{
	const State &stateValue = m_StateValues.at( image );
	GLdouble win[3];
	GLdouble pro[16];
	gluProject( objx, objy, 0, stateValue.modelViewMatrix, stateValue.projectionMatrix, stateValue.viewport, &win[0], &win[1], &win[2] );
	return std::make_pair<int16_t, int16_t>( ( win[0] - stateValue.viewport[0] ), win[1] - stateValue.viewport[1] );
}


bool QGLWidgetImplementation::calculateTranslation(  )
{
	State state = m_StateValues[m_ViewerCore->getCurrentImage()];
	std::pair<int16_t, int16_t> center = std::make_pair<int16_t, int16_t>( abs( state.mappedImageSize[0] ) / 2, abs( state.mappedImageSize[1] ) / 2 );
	float shiftX = center.first - ( state.mappedVoxelCoords[0] < 0 ? abs( state.mappedImageSize[0] ) + state.mappedVoxelCoords[0] : state.mappedVoxelCoords[0] );
	float shiftY =  center.second - ( state.mappedVoxelCoords[1] < 0 ? abs( state.mappedImageSize[1] ) + state.mappedVoxelCoords[1] : state.mappedVoxelCoords[1] );
	shiftX = ( 1.0 / abs( state.mappedImageSize[0] ) ) * shiftX ;
	shiftY = ( 1.0 / abs( state.mappedImageSize[1] ) ) * shiftY ;
	float zoomDependentShift = 1.0 - ( 2.0 / m_Zoom.currentZoom );
	BOOST_FOREACH( StateMap::reference stateRef, m_StateValues )
	{
		stateRef.second.modelViewMatrix[12] = shiftX + zoomDependentShift * shiftX;
		stateRef.second.modelViewMatrix[13] = shiftY + zoomDependentShift * shiftY;
	}
}

bool QGLWidgetImplementation::lookAtPhysicalCoords( const boost::shared_ptr< ImageHolder > image, const isis::util::fvector4 &physicalCoords )
{
	lookAtVoxel( image, image->getImage()->getIndexFromPhysicalCoords( physicalCoords ) );
}

bool QGLWidgetImplementation::lookAtPhysicalCoords( const isis::util::fvector4 &physicalCoords )
{
	redraw();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	BOOST_FOREACH( StateMap::const_reference state, m_StateValues ) 
	{
		updateStateValues(  state.first, state.first->getImage()->getIndexFromPhysicalCoords( physicalCoords ) );
	}
	
	BOOST_FOREACH( StateMap::const_reference state, m_StateValues ) 
	{
		if( state.first->getImageState().visible ) {
			paintScene(  state.first );
		}
	}

	if( m_StateValues.size() ) {
		paintCrosshair();
	}
}


bool QGLWidgetImplementation::lookAtVoxel( const isis::util::ivector4 &voxelCoords )
{
	LOG( Debug, verbose_info ) << "Looking at voxel: " << voxelCoords;
	//someone has told the widget to paint all available images.
	//So first we have to update the state values for each image
	redraw();
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	BOOST_FOREACH( StateMap::const_reference state, m_StateValues ) 
	{
		updateStateValues(  state.first, voxelCoords );
	}
	BOOST_FOREACH( StateMap::const_reference state, m_StateValues ) 
	{
		if( state.first->getImageState().visible ) {
			paintScene(  state.first );
		}
	}

	if( m_StateValues.size() ) {
		paintCrosshair();
	}
}

bool QGLWidgetImplementation::lookAtVoxel( const boost::shared_ptr<ImageHolder> image, const util::ivector4 &voxelCoords )
{
	updateStateValues( image, voxelCoords );
	if( image->getImageState().visible ) {
		paintScene( image );
	}
	if( m_StateValues.size() ) {
		paintCrosshair();
	}
}


void QGLWidgetImplementation::paintScene( const boost::shared_ptr<ImageHolder> image )
{

	const State &state = m_StateValues[image];

	double scaling, bias;

	if( m_ScalingType == automatic_scaling ) {
		scaling = image->getOptimalScalingPair().second;
		bias = image->getOptimalScalingPair().first;
	} else if ( m_ScalingType == manual_scaling ) {
		scaling = m_ScalingPair.second;
		bias = m_ScalingPair.first;
	} else {
		scaling = 1.0;
		bias = 0.0;
	}
	glViewport( state.viewport[0], state.viewport[1], state.viewport[2], state.viewport[3] );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glLoadMatrixd( state.projectionMatrix );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glLoadMatrixd( state.modelViewMatrix );
	glMatrixMode( GL_TEXTURE );
	glLoadIdentity();
	glLoadMatrixd( state.textureMatrix );
	//shader

	//if the image is declared as a zmap
	if( image->getImageState().imageType == ImageHolder::z_map ) {
		m_ScalingShader.setEnabled( false );
		m_LUTShader.setEnabled( true );
		GLuint id = m_LookUpTable.getLookUpTableAsTexture( Color::hsvLUT_reverse );
		glActiveTexture( GL_TEXTURE1 );
		glBindTexture( GL_TEXTURE_1D, id );
		m_LUTShader.addVariable<float>( "lut", 1, true );
		m_LUTShader.addVariable<float>( "max", image->getMinMax().second->as<float>() );
		m_LUTShader.addVariable<float>( "min", image->getMinMax().first->as<float>() );
		m_LUTShader.addVariable<float>( "killZeros", 1.0 );
		m_LUTShader.addVariable<float>( "upper_threshold", image->getImageState().threshold.second );
		m_LUTShader.addVariable<float>( "lower_threshold", image->getImageState().threshold.first );
		m_LUTShader.addVariable<float>( "bias", 0.0 );
		m_LUTShader.addVariable<float>( "scaling", 1.0 );
		m_LUTShader.addVariable<float>( "opacity", image->getImageState().opacity );
	} else if ( image->getImageState().imageType == ImageHolder::anatomical_image ) {
		m_ScalingShader.setEnabled( true );
		m_ScalingShader.addVariable<float>( "max", image->getMinMax().second->as<float>() );
		m_ScalingShader.addVariable<float>( "min", image->getMinMax().first->as<float>() );
		m_ScalingShader.addVariable<float>( "upper_threshold",  image->getImageState().threshold.second );
		m_ScalingShader.addVariable<float>( "lower_threshold", image->getImageState().threshold.first );
		m_ScalingShader.addVariable<float>( "scaling", scaling );
		m_ScalingShader.addVariable<float>( "bias", bias );
		m_ScalingShader.addVariable<float>( "opacity", image->getImageState().opacity );
	}

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_3D, state.textureID );

	glBegin( GL_QUADS );
	glTexCoord3f( 0, 0, state.normalizedSlice );
	glVertex2f( -1.0, -1.0 );
	glTexCoord3f( 0, 1, state.normalizedSlice );
	glVertex2f( -1.0, 1.0 );
	glTexCoord3f( 1, 1, state.normalizedSlice );
	glVertex2f( 1.0, 1.0 );
	glTexCoord3f( 1, 0, state.normalizedSlice );
	glVertex2f( 1.0, -1.0 );
	glEnd();
	glDisable( GL_TEXTURE_3D );

	m_ScalingShader.setEnabled( false );
	m_LUTShader.setEnabled( false );

}

void QGLWidgetImplementation::paintCrosshair()
{
	//paint crosshair
	glDisable( GL_BLEND );
	const State &currentState = m_StateValues.at( m_ViewerCore->getCurrentImage() );
	glColor4f( 1, 1, 1, 1 );
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

	if( m_ShowLabels ) {
		viewLabels();
	}

	redraw();
}



void QGLWidgetImplementation::viewLabels()
{
	QFont font;
	font.setPointSize( 15 );
	font.setPixelSize( 15 );
	glColor4f( 0.0, 1.0, 1.0, 1.0 );
	glViewport( 0, 0, width(), height() );

	switch( m_PlaneOrientation ) {
	case axial:
		glColor4f( 0.0, 1.0, 0.0, 1.0 );
		renderText( width() / 2 - 7, 25, QString( "A" ), font );
		renderText( width() / 2 - 7, height() - 10, QString( "P" ), font );
		glColor4f( 1.0, 0.0, 0.0, 1.0 );
		renderText( 5, height() / 2, QString( "L" ), font );
		renderText( width() - 15, height() / 2, QString( "R" ), font );
		break;
	case sagittal:
		glColor4f( 0.0, 1.0, 0.0, 1.0 );
		renderText( 5, height() / 2, QString( "A" ), font );
		renderText( width() - 15, height() / 2, QString( "P" ), font );
		glColor4f( 0.0, 0.0, 1.0, 1.0 );
		renderText( width() / 2 - 7, 25, QString( "S" ), font );
		renderText( width() / 2 - 7, height() - 10, QString( "I" ), font );
		break;
	case coronal:
		glColor4f( 0.0, 0.0, 1.0, 1.0 );
		renderText( width() / 2 - 7, 25, QString( "S" ), font );
		renderText( width() / 2 - 7, height() - 10, QString( "I" ), font );
		glColor4f( 1.0, 0.0, 0.0, 1.0 );
		renderText( 5, height() / 2, QString( "L" ), font );
		renderText( width() - 15, height() / 2, QString( "R" ), font );
		break;
	}
}


void QGLWidgetImplementation::mouseMoveEvent( QMouseEvent *e )
{
	if ( rightButtonPressed || leftButtonPressed ) {
		emitMousePressEvent( e );
	}
}

void QGLWidgetImplementation::mousePressEvent( QMouseEvent *e )
{
	if( e->button() == Qt::LeftButton ) {
		leftButtonPressed = true;
	}

	if( e->button() == Qt::RightButton ) {
		rightButtonPressed = true;
	}

	emitMousePressEvent( e );

}

bool QGLWidgetImplementation::isInViewport( size_t wx, size_t wy )
{
	GLint *viewport = m_StateValues[m_ViewerCore->getCurrentImage()].viewport;
	if( ( wx > viewport[0] && wx < ( viewport[0] + viewport[2] ) ) && ( wy > viewport[1] && wy < ( viewport[1] + viewport[3] ) ) ) {
		return true;
	} else {
		return false;
	}
}


void QGLWidgetImplementation::emitMousePressEvent( QMouseEvent *e )
{
	if( isInViewport( e->x(), height() - e->y() ) ) {
		std::pair<float, float> objectCoords = window2ObjectCoords( e->x(), height() - e->y(), m_ViewerCore->getCurrentImage() );
		util::ivector4 voxelCoords = GLOrientationHandler::transformObject2VoxelCoords( util::fvector4( objectCoords.first, objectCoords.second, m_StateValues.at( m_ViewerCore->getCurrentImage() ).normalizedSlice ), m_ViewerCore->getCurrentImage(), m_PlaneOrientation );
		physicalCoordsChanged( m_ViewerCore->getCurrentImage()->getImage()->getPhysicalCoordsFromIndex( voxelCoords ) );
	}
}

bool QGLWidgetImplementation::timestepChanged( unsigned int timestep )
{
	BOOST_FOREACH( StateMap::reference currentImage, m_StateValues ) {
		if( currentImage.first->getImageSize()[3] > timestep ) {
			currentImage.second.voxelCoords[3] = timestep;
		} else {
			currentImage.second.voxelCoords[3] = currentImage.first->getImageSize()[3] - 1;
		}

		updateScene();
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
		BOOST_FOREACH( StateMap::reference state, m_StateValues ) {
			glMatrixMode( GL_PROJECTION );
			glLoadMatrixd( state.second.projectionMatrix );
			glScalef( zoomFactor, zoomFactor, 1 );
			glGetDoublev( GL_PROJECTION_MATRIX, state.second.projectionMatrix );
			glLoadIdentity();
		}
		zoomEventHappened = true;
		updateScene();
	} else {
		m_Zoom.currentZoom = 1;
	}

}

void QGLWidgetImplementation::mouseReleaseEvent( QMouseEvent *e )
{
	if( e->button() == Qt::LeftButton ) {
		leftButtonPressed = false;
	}

	if( e->button() == Qt::RightButton ) {
		rightButtonPressed = false;
	}
}


void QGLWidgetImplementation::keyPressEvent( QKeyEvent *e )
{
	if( e->key() == Qt::Key_Space ) {
		BOOST_FOREACH( StateMap::reference ref, m_StateValues ) {
			GLOrientationHandler::makeIdentity( ref.second.modelViewMatrix );
			GLOrientationHandler::makeIdentity( ref.second.projectionMatrix );
		}
		m_Zoom.currentZoom = 1.0;
		size_t timestep = m_StateValues[m_ViewerCore->getCurrentImage()].voxelCoords[3];
		util::ivector4 size = m_ViewerCore->getCurrentImage()->getImageSize();
		lookAtPhysicalCoords( m_ViewerCore->getCurrentImage()->getImage()->getPhysicalCoordsFromIndex( util::ivector4( size[0] / 2, size[1] / 2, size[2] / 2 ) ) );
	}
}

void  QGLWidgetImplementation::setShowLabels( bool show )
{
	m_ShowLabels = show;
	updateScene();

}

void QGLWidgetImplementation::setInterpolationType( const isis::viewer::GLTextureHandler::InterpolationType interpolation )
{
	m_InterplationType = interpolation;
	updateScene();
}

void QGLWidgetImplementation::updateScene()
{
	lookAtPhysicalCoords( m_ViewerCore->getCurrentImage()->getImage()->getPhysicalCoordsFromIndex( m_StateValues[m_ViewerCore->getCurrentImage()].voxelCoords ) );
}


}
} // end namespace