#include "ViewerCoreBase.hpp"

namespace isis
{
namespace viewer
{

ViewerCoreBase::ViewerCoreBase( )
	: m_CurrentTimestep( 0 )
{
}



void ViewerCoreBase::addImageList( const std::list< data::Image > imageList, const ImageHolder::ImageType &imageType, const util::slist &filenames )
{
	bool ignoreFilenames = false;
	if( filenames.size() != imageList.size() )
	{
		ignoreFilenames = true;
		LOG( Runtime, error ) << "The size of the image list does not coincide with the amount of filenames. Ignoring filenames.";
	}
	util::slist::const_iterator filenameIterator = filenames.begin();
	if( !imageList.empty() ) {
		BOOST_FOREACH( std::list< data::Image >::const_reference imageRef, imageList ) {
			if(!ignoreFilenames) {
				m_DataContainer.addImage( imageRef, imageType, *(filenameIterator++) );
			} else {
				m_DataContainer.addImage( imageRef, imageType );
			}
		}
	} else {
		LOG( Runtime, warning ) << "The image list passed to the core is empty!";
	}
	m_CurrentImage = getDataContainer()[m_DataContainer.size() - 1];

}



void ViewerCoreBase::setImageList( const std::list< data::Image > imageList, const ImageHolder::ImageType &imageType, const util::slist &filenames )
{
	if( !imageList.empty() ) {
		m_DataContainer.clear();
	}
	ViewerCoreBase::addImageList( imageList, imageType, filenames );
}


}
} // end namespace