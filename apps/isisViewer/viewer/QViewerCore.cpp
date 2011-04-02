#include "QViewerCore.hpp"

namespace isis
{
namespace viewer
{

QViewerCore::QViewerCore( data::Image image ): ViewerCoreBase( image )
{
	
}


bool
QViewerCore::registerWidget( std::string key, QWidget *widget )
{
	if( m_WidgetMap.find( key ) == m_WidgetMap.end() ) {
		widget->setObjectName( QString( key.c_str() ) );
		m_WidgetMap.insert( std::make_pair< std::string,  QWidget * >( key, widget ) );
	} else {
		LOG( Runtime, error ) << "A widget with the name " << key << " already exists! Wont add this";
		return false;
	}
}

void QViewerCore::voxelCoordChanged(util::ivector4 )
{

}


}
}