#ifndef VIEWERCORE_HPP
#define VIEWERCORE_HPP

#include "ViewerCoreBase.hpp"


namespace isis
{
namespace viewer
{

class ViewerCore : public ViewerCoreBase
{
public:
	boost::weak_ptr<void>
	getImageWeakPointer( unsigned short imageID = 0, unsigned int timestep = 0 ) {
		LOG_IF( getDataContainer().size() < imageID, Runtime, error ) << "There is no image with iamgeID " << imageID << "!";
		return getDataContainer()[imageID].getImageVector()[timestep]->getRawAddress();
	}


};


}
} // end namespace






#endif
