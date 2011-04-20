#include "DataContainer.hpp"

namespace isis
{
namespace viewer
{

bool DataContainer::addImage( const data::Image &image, const ImageHolder::ImageType &imageType, const std::string &filename )
{
	std::string newFileName = filename;
	if(find(filename) != end() ) {
		size_t index = 0;
		std::cout << "gna" << std::endl;
		while (find(newFileName) != end() ) {
			std::stringstream ss;
			ss << filename << " (" << ++index << ")";
			newFileName = ss.str();
			std::cout << newFileName << std::endl;
		}
	}
	ImageHolder tmpHolder;
	tmpHolder.setImage( image, imageType, newFileName );
	tmpHolder.setID( size() );
	//we have to check if the filename already exists and if so adding 
		
	
	insert( std::make_pair<std::string, ImageHolder>( newFileName, tmpHolder ) );

	return true;
}


}
} // end namespace