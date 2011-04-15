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
class DataContainer : public std::vector<ImageHolder>
{
public:
	typedef std::map< std::pair<std::string, ImageHolder::ImageType>, ImageHolder> ImageFileMapType;
	
	///simply adds an isis image to the vector
	bool addImage( const data::Image &image, const ImageHolder::ImageType &imageType, const std::string &filename = "" );

	///checks if a image with the given parameters exists
	bool isImage( size_t imageID, size_t timestep = 0, size_t slice = 0 ) const;

	///returns a boost::weak_ptr of the images data. Actually this also is a convinient function.
	boost::weak_ptr<void>
	getImageWeakPointer( const ImageHolder &image, size_t timestep = 0 ) const {
		return image.getImageVector()[timestep]->getRawAddress();
	}
	
	ImageFileMapType getFileNameMap() const { return m_FileNameMap; }
	
private:
	ImageFileMapType m_FileNameMap;

};


}
} // end namespace

#endif