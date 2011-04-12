#include "ViewerCoreBase.hpp"

namespace isis
{
namespace viewer
{

ViewerCoreBase::ViewerCoreBase( )
	: m_CurrentTimestep( 0 )
{
}



void ViewerCoreBase::addImageList( const std::list< data::Image > imageList, const util::slist &filenames )
{
	if( !imageList.empty() ) {
		BOOST_FOREACH( std::list< data::Image >::const_reference imageRef, imageList ) {
			m_DataContainer.addImage( imageRef );
		}
	} else {
		LOG( Runtime, warning ) << "The image list passed to the core is empty!";
	}
	m_CurrentImage = getDataContainer()[m_DataContainer.size() - 1];

}



void ViewerCoreBase::setImageList( const std::list< data::Image > imageList, const util::slist &filenames )
{
	if( !imageList.empty() ) {
		m_DataContainer.clear();
	}
	addImageList( imageList, filenames );
}


}
} // end namespace