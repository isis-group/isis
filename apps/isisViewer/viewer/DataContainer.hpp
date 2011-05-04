#ifndef DATACONTAINER_HPP
#define DATACONTAINER_HPP

#include <vector>
#include "ImageHolder.hpp"

namespace isis
{
namespace viewer
{

/**
 * This class is a std::vector which holds all images as an object of ImageHolder.
 * Furthermore it provides some convinient functions to add images and getting the
 * pointer to the image data.
 */
class DataContainer : public std::map<std::string, boost::shared_ptr<ImageHolder> >
{
public:
	///simply adds an isis image to the vector
	bool addImage( const data::Image &image, const ImageHolder::ImageType &imageType, const std::string &filename = "" );

	///returns a boost::weak_ptr of the images data. Actually this also is a convinient function.
	boost::weak_ptr<void>
	getImageWeakPointer( const boost::shared_ptr<ImageHolder> image, size_t timestep = 0 ) const {
		return image->getImageVector()[timestep]->getRawAddress();
	}


};


}
} // end namespace

#endif