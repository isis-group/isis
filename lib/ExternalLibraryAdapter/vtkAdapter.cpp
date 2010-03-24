#include "vtkAdapter.hpp"

namespace isis{ namespace adapter{
    
VTKAdapter::VTKAdapter(const boost::shared_ptr<isis::data::Image> src )
    : vtkImageData(), dimensions(src->sizeToVector())
{
    //go through all the chunks and check for consistent data type
    unsigned int firstTypeID = src->chunksBegin()->typeID();
    for (data::Image::ChunkIterator ci = src->chunksBegin();ci != src->chunksEnd(); *ci++)
    {
	if(not ci->typeID() == firstTypeID)
	{
	    LOG(DataLog, error) << "Inconsistent ";
	}
	
    }
    
    
    
    
    
    
}

VTKAdapter::VTKAdapter(const isis::data::ImageList& src)
    : vtkImageData()
{
}

}} //end namespace