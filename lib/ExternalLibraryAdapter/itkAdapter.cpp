#include "itkAdapter.hpp"

namespace isis{ namespace adapter{
    
ITKAdapter::ITKAdapter(const boost::shared_ptr<isis::data::Image> src)
    
{}
template<typename T,unsigned short dim> itk::SmartPointer<itk::Image<T,dim> > 
		ITKAdapter::makeItkImage(const boost::shared_ptr<data::Image> src, const ChunkArrangement& arragement)
{
}

}}// end namespace