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
	m_ImageMap[filename] = tmpHolder;
	push_back( tmpHolder );
	
	return true;
}


ImageHolder &DataContainer::getImageHolder(const std::string& filename)
{
	if( m_ImageMap.find(filename) != m_ImageMap.end()) {
		return m_ImageMap.find(filename)->second;
	} else {
		LOG(Runtime, warning) << "No image with filename " << filename << " was found.";
	}
}



}
} // end namespace