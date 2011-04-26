#include "DataContainer.hpp"

namespace isis
{
namespace viewer
{

bool DataContainer::addImage( const data::Image &image, const ImageHolder::ImageType &imageType, const std::string &filename )
{
	std::string newFileName = filename;

	if( find( filename ) != end() ) {
		size_t index = 0;

		while ( find( newFileName ) != end() ) {
			std::stringstream ss;
			ss << filename << " (" << ++index << ")";
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