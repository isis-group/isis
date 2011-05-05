#include "DataContainer.hpp"

namespace isis
{
namespace viewer
{

bool DataContainer::addImage( const data::Image &image, const ImageHolder::ImageType &imageType )
{
	std::string fileName;
	if( image.hasProperty("source") ) {
		fileName = image.getPropertyAs<std::string>("source");
	} else {
		boost::filesystem::path path = image.getChunk(0).getPropertyAs<std::string>("source");
		fileName = path.branch_path().string();
	}
	std::string newFileName = fileName;
	if( find( fileName ) != end() ) {
		size_t index = 0;
		while ( find( newFileName ) != end() ) {
			std::stringstream ss;
			ss << fileName << " (" << ++index << ")";
			newFileName = ss.str();
		}
	}
	boost::shared_ptr<ImageHolder>  tmpHolder = boost::shared_ptr<ImageHolder> ( new ImageHolder ) ;
	tmpHolder->setImage( image, imageType, newFileName );
	tmpHolder->setID( size() );
	insert( std::make_pair<std::string, boost::shared_ptr<ImageHolder> >( newFileName, tmpHolder ) );

	return true;
}


}
} // end namespace