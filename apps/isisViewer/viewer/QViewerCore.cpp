#include "QViewerCore.hpp"

namespace isis
{
namespace viewer
{

QViewerCore::QViewerCore( data::Image image ): ViewerCoreBase( image )
{
}



bool
QViewerCore::registerWidget( std::string key, QWidget *widget, QViewerCore::Actions action )
{
	if( m_WidgetMap.find( key ) == m_WidgetMap.end() ) {
		widget->setObjectName( QString( key.c_str() ) );
		m_WidgetMap.insert( std::make_pair< std::string,  QWidget * >( key, widget ) );

		if( dynamic_cast<QGLWidgetImplementation *>( widget ) ) {
			connect( dynamic_cast<QGLWidgetImplementation *>( widget ), SIGNAL( voxelCoordChanged( util::ivector4 ) ), this, SLOT( voxelCoordChanged ( util::ivector4 ) ) );
			connect( this, SIGNAL( emitVoxelCoordChanged( util::ivector4 ) ), dynamic_cast<QGLWidgetImplementation *>( widget ), SLOT( lookAtVoxel( util::ivector4 ) ) );
			connect( this, SIGNAL( emitTimeStepChange( unsigned int ) ), dynamic_cast<QGLWidgetImplementation *>( widget ), SLOT( timestepChanged( unsigned int ) ) );
		} else if ( dynamic_cast< QSpinBox * >( widget ) ) {
			switch ( action ) {
			case QViewerCore::timestep_changed:
				connect( dynamic_cast<QSpinBox *>( widget ), SIGNAL( valueChanged( int ) ), this, SLOT( timestepChanged( int ) ) );
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
	Q_EMIT emitVoxelCoordChanged( voxelCoords );
}

void QViewerCore::timestepChanged( int timestep )
{
	Q_EMIT emitTimeStepChange( timestep );
}

}
}