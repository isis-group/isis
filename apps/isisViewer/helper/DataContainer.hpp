#ifndef DATACONTAINER_HPP
#define DATACONTAINER_HPP

#include <vector>
#include "ImageHolder.hpp"

namespace isis
{
namespace viewer
{

class DataContainer : public std::vector<ImageHolder>
{
public:
	bool addImage( const data::Image & );
	boost::weak_ptr<void>
	getImageWeakPointer( size_t imageID = 0, size_t timestep = 0 ) const {
		LOG_IF( size() < imageID, Runtime, error ) << "There is no image with iamgeID " << imageID << "!";
		return operator[](imageID).getImageVector()[timestep]->getRawAddress();
	}

};


}
} // end namespace

#endif