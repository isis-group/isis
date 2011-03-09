#include "ViewerCoreBase.hpp"

namespace isis {
namespace viewer {

void ViewerCoreBase::setImageList( const std::list< data::Image > imageList )
{
	if( !imageList.empty()) {
		m_DataContainer.clear();
		BOOST_FOREACH( std::list< data::Image >::const_reference imageRef, imageList )
		{
			m_DataContainer.addImage( imageRef );
		}
	} else {
		LOG( Runtime, warning ) << "The image list passed to the core is empty!";
	}
}

bool 
ViewerCoreBase::registerWidget( std::string key, boost::shared_ptr<WidgetAdapterBase> widget )
{
	if( m_WidgetMap.find( key ) == m_WidgetMap.end() ){
		m_WidgetMap.insert( std::make_pair< std::string, boost::shared_ptr< WidgetAdapterBase > >(key, widget) );
	} else {
		LOG( Runtime, error ) << "A widget with the name " << key << " already exists! Wont add this";
		return false;
	}
}	

}} // end namespace