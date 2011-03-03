#include "ImageHolder.hpp"

namespace isis {
namespace viewer {
	
template<class TYPE>
ImageHolder<TYPE>::ImageHolder()
	: m_NumberOfTimeSteps(0),
	m_IsValid(false)
{

}

template<class TYPE>
bool
ImageHolder<TYPE>::setImage( const data::Image &image ) {
	//here we have to check if the type of the chunks is 
	
	m_ImageSize = image.getSizeAsVector();
	m_TypeID = image.getMajorTypeID();
	m_NumberOfTimeSteps = m_ImageSize[3];

	BOOST_FOREACH(ChunkVector::const_reference chRef, image.getChunksAsVector())
	{
		
	}
	
}
}} //end namespace
