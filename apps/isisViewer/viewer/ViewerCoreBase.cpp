#include "ViewerCoreBase.hpp"

namespace isis
{
namespace viewer
{

ViewerCoreBase::ViewerCoreBase( )
	: m_CurrentTimestep( 0 ),
	  m_AllImagesToIdentity( false )

{
}

bool ViewerCoreBase::setAllImagesToIdentity( bool identity )
{
	BOOST_FOREACH( DataContainer::reference images, m_DataContainer ) {

	}
}


void ViewerCoreBase::addImageList( const std::list< data::Image > imageList, const ImageHolder::ImageType &imageType )
{
	if( !imageList.empty() ) {
		BOOST_FOREACH( std::list< data::Image >::const_reference imageRef, imageList ) {
			m_DataContainer.addImage( imageRef, imageType );
		}
		setCurrentImage( m_DataContainer.begin()->second );
	} else {
		LOG( Runtime, warning ) << "The image list passed to the core is empty!";
	}



}



void ViewerCoreBase::setImageList( std::list< data::Image > imgList, const ImageHolder::ImageType &imageType )
{
	if( !imgList.empty() ) {
		m_DataContainer.clear();
	}

	ViewerCoreBase::addImageList( imgList, imageType );
}



}
} // end namespace