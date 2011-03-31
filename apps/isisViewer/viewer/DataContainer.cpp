#include "DataContainer.hpp"

namespace isis
{
namespace viewer
{

bool DataContainer::addImage( const data::Image &image )
{
	ImageHolder tmpHolder(image);
	push_back( tmpHolder );
	return true;
}


bool DataContainer::isImage( size_t imageID, size_t timestep, size_t slice ) const
{
	if ( size() < imageID + 1 ) {
		return false;
	} else if ( operator[]( imageID ).getImageVector().size() < timestep + 1 ) {
		return false;
	}

	return true;

}

}
} // end namespace