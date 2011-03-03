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
ImageHolder<TYPE>::setImage( data::Image image ) 
{
	//some checks
	if( image.isEmpty() ) {
		LOG( Runtime, error) << "Getting an empty image? Obviously something went wrong.";
		return false;
	}
	//check if the TYPE corresponds to the typeID of the image
	m_TypeID = image.getMajorTypeID();
	if( data::ValuePtr<TYPE>::staticID != m_TypeID ) {
		LOG(  Runtime, error ) << "The type of the image (" 
			<< image.getMajorTypeName() << ") is different than the templated type (" 
			<< data::ValuePtr<TYPE>::staticName() << ")! ";
		return false;
	}

	// get some image information
	m_ImageSize = image.getSizeAsVector();
	m_NumberOfTimeSteps = m_ImageSize[3];
	LOG( Debug, verbose_info)  << "Fetched image of size " << m_ImageSize << " and type "
		<< image.getMajorTypeName() << ".";
	
	//copy the image into continuous memory space and assure consistent data type
	data::ValuePtr<TYPE> imagePtr( ( TYPE* ) calloc( image.getVolume(), sizeof(TYPE) ), image.getVolume() );
	image.copyToMem<TYPE>( &imagePtr[0] );
	LOG( Debug, verbose_info) << "Copied image to continuous memory space.";
	//splice the image in its volumes -> we get a vector of t volumes
	m_ImageVector = imagePtr.splice( m_ImageSize[0] * m_ImageSize[1] * m_ImageSize[2] );
	LOG_IF( m_ImageVector.empty(), Runtime, error ) << "Size of image vector is 0!";
	if(m_ImageVector.size() != m_NumberOfTimeSteps) {
		LOG( Runtime, error) << "The number of timesteps (" << m_NumberOfTimeSteps 
		<< ") does not coincide with the number of volumes ("  << m_ImageVector.size() << ").";
		return false;
	}
	LOG(Debug, verbose_info) << "Spliced image to " << m_ImageVector.size() << " volumes.";
	
	//copy all the relevant meta information
	m_PropMap = static_cast<util::PropertyMap>( image );
	
	//workaround cause compiles do not understand to use getChunksAsVector directly in BOOST_FOREACH
	_internal::ChunkVector chVec = image.getChunksAsVector();
	BOOST_FOREACH( _internal::ChunkVector::const_reference chRef, chVec)
	{
		m_ChunkProperties.push_back( static_cast<util::PropertyMap>(*chRef) );
	}
	LOG( Debug, verbose_info ) << "Fetched " << m_ChunkProperties.size() << " chunk properties.";
	//image seems to be ok...i guess
	return filterRelevantMetaInformation(); //only return true if filtering is successfully
}

template<typename TYPE>
bool
ImageHolder<TYPE>::filterRelevantMetaInformation()
{
	// in case we get more chunks than timesteps we should filter the chunk metadata
	if( m_ChunkProperties.size() > m_NumberOfTimeSteps )
	{
		if(m_ChunkProperties.size() % m_NumberOfTimeSteps) {
			LOG( Runtime, warning ) << "Cannot filter the metadata for each timestep. Your image contains of " 
				<< m_ChunkProperties.size() << " chunks and " << m_NumberOfTimeSteps 
				<< " timesteps. The number of chunks should be a multiple of number of timesteps!";
			return false;
		} else {
			size_t factor = m_ChunkProperties.size() / m_NumberOfTimeSteps;
			for ( size_t t = 0; t<m_ChunkProperties.size(); t += factor ) {
				m_TimeStepProperties.push_back( m_ChunkProperties[t] );
			}
			if( m_TimeStepProperties.size() != m_NumberOfTimeSteps ) {
				LOG(  Runtime, warning ) << "Something went wrong with filtering the properties of each timestep. You got " 
					<< m_TimeStepProperties.size() << " timestep properties for " << m_NumberOfTimeSteps << " timestep.";
				return false;
			}				
		}
	} 
	return true;
}

}} //end namespace
