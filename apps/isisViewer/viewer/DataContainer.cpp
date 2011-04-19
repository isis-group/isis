#include "DataContainer.hpp"

namespace isis
{
namespace viewer
{

bool DataContainer::addImage( const data::Image &image, const ImageHolder::ImageType &imageType, const std::string &filename )
{
	ImageHolder tmpHolder;
	tmpHolder.setImage( image, imageType, filename );
	tmpHolder.setID( size() );
	insert( std::make_pair<std::string, ImageHolder>( filename, tmpHolder ) );

	return true;
}


}
} // end namespace