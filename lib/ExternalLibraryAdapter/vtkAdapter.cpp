#include "vtkAdapter.hpp"

namespace isis{
    
VTKAdapter::VTKAdapter(const boost::shared_ptr<isis::data::Image> src )
    : vtkImageData()
{
}

VTKAdapter::VTKAdapter(const isis::data::ImageList& src)
    : vtkImageData()
{
}

} //end namespace isis