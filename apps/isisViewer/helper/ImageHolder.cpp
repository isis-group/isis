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
ImageHolder<TYPE>::setImage( const data::Image &image ) 
{
	//some checks
	LOG_IF( image.isEmpty(), Runtime, error) << "Getting an empty image? Obviously something went wrong.";
	assert( !image.isEmpty() );
	//check if the TYPE corersponds to the typeID of the image
	m_TypeID = image.getMajorTypeID();
	LOG_IF( data::ValuePtr<TYPE>::staticID != m_TypeID, Runtime, error ) << "The type of the image (" 
		<< image.getMajorTypeName() << ") is different than the templated type (" 
		<< data::ValuePtr<TYPE>::staticName() << ")! ";
	assert( data::ValuePtr<TYPE>::staticID == m_TypeID );

	// get some image information
	m_ImageSize = image.getSizeAsVector();
	m_NumberOfTimeSteps = m_ImageSize[3];
	LOG( Runtime, verbose_info)  << "Fetched image of size " << m_ImageSize << ".";
	
	//first we have to copy and if necessary convert the image to a continuous memory space
	TYPE *imagePtr = calloc(image.getVolume(), sizeof(TYPE) );
	image.copyToMem<TYPE>(imagePtr);
	LOG( Runtime, verbose_info) << "Copied image to continuous memory space.";
	assert( imagePtr != NULL );
	//now we can iterate over the timesteps and create a boost::shared_ptr for each timestep	
	size_t volumeSize = m_ImageSize[0] * m_ImageSize[1] * m_ImageSize[2] * sizeof(TYPE);
	for (size_t t = 0; t < m_NumberOfTimeSteps; t++)
	{
		
	}
	
	
}
}} //end namespace
