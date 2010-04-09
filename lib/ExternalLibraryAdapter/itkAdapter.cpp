#include "itkAdapter.hpp"

namespace isis{ namespace adapter{
    
itkAdapter::itkAdapter(const boost::shared_ptr<isis::data::Image> src)
    
{}
template<typename T,unsigned short dim> itk::SmartPointer<itk::Image<T,dim> > 
		itkAdapter::makeItkImage(const boost::shared_ptr<data::Image> src)
{
}

}}// end namespace