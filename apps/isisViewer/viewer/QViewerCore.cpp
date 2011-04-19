#include "QViewerCore.hpp"


namespace isis
{
namespace viewer
{

QViewerCore::QViewerCore( ): ViewerCoreBase( )
{
}



bool
QViewerCore::registerWidget( std::string key, QWidget *widget, QViewerCore::Actions action )
{
	if( m_WidgetMap.find( key ) == m_WidgetMap.end() ) {
		widget->setObjectName( QString( key.c_str() ) );
		m_WidgetMap.insert( std::make_pair< std::string,  QWidget * >( key, widget ) );

		if( dynamic_cast<QGLWidgetImplementation *>( widget ) ) {
			QGLWidgetImplementation *w = dynamic_cast<QGLWidgetImplementation *>( widget );
			connect( w, SIGNAL( voxelCoordChanged( util::ivector4 ) ), this, SLOT( voxelCoordChanged ( util::ivector4 ) ) );
			connect( this, SIGNAL( emitVoxelCoordChanged( util::ivector4 ) ), w, SLOT( lookAtVoxel( util::ivector4 ) ) );
			connect( this, SIGNAL( emitTimeStepChange( unsigned int ) ), w, SLOT( timestepChanged( unsigned int ) ) );
			connect( this, SIGNAL( emitShowLabels( bool ) ), w, SLOT( setShowLabels( bool ) ) );
			connect( this, SIGNAL( emitUpdateScene() ), w, SLOT( updateScene() ) );
		} else if ( dynamic_cast< QSpinBox * >( widget ) ) {
			switch ( action ) {
			case QViewerCore::timestep_changed:
				connect( dynamic_cast<QSpinBox *>( widget ), SIGNAL( valueChanged( int ) ), this, SLOT( timestepChanged( int ) ) );
				break;
			}
		} else if ( dynamic_cast< QCheckBox * > ( widget ) ) {
			switch ( action ) {
			case QViewerCore::show_labels:
				connect( dynamic_cast<QCheckBox *>( widget ), SIGNAL( stateChanged( int ) ), this, SLOT( setShowLabels( int ) ) );
				break;
			}
		}


	} else {
		LOG( Runtime, error ) << "A widget with the name " << key << " already exists! Wont add this";
		return false;
	}
}

void QViewerCore::voxelCoordChanged( util::ivector4 voxelCoords )
{
	emitVoxelCoordChanged( voxelCoords );
}

void QViewerCore::timestepChanged( int timestep )
{
	emitTimeStepChange( timestep );
}

void QViewerCore::addImageList( const std::list< data::Image > imageList, const ImageHolder::ImageType &imageType, const isis::util::slist &filenames )
{
	isis::viewer::ViewerCoreBase::addImageList( imageList, imageType, filenames );
	emitImagesChanged( getDataContainer() );
}

void QViewerCore::setImageList( const std::list< data::Image > imageList, const ImageHolder::ImageType &imageType, const isis::util::slist &filenames )
{
	isis::viewer::ViewerCoreBase::setImageList( imageList, imageType, filenames );
	emitImagesChanged( getDataContainer() );
}

void QViewerCore::setShowLabels( bool l )
{
	if( l ) {
		emitShowLabels( true );
	} else {
		emitShowLabels( false );
	}
}

void QViewerCore::updateScene()
{
	emitUpdateScene();
}



}
}