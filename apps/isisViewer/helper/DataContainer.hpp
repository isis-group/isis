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
  
	///simply adds an isis image to the vector
	bool addImage( const data::Image & );
	
	///returns a boost::weak_ptr of the images data. Actually this also is a convinient function.
	boost::weak_ptr<void>
	getImageWeakPointer( size_t imageID = 0, size_t timestep = 0 ) const {
		LOG_IF( size() < imageID, Runtime, error ) << "There is no image with iamgeID " << imageID << "!";
		return operator[]( imageID ).getImageVector()[timestep]->getRawAddress();
	}

};


}
} // end namespace

#endif