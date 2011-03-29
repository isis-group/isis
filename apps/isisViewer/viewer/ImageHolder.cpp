
#include "ImageHolder.hpp"

namespace isis
{
namespace viewer
{

ImageHolder::ImageHolder()
	: m_NumberOfTimeSteps( 0 )
{}



bool
ImageHolder::filterRelevantMetaInformation()
{
	// in case we get more chunks than timesteps we should filter the chunk metadata
	if( m_ChunkProperties.size() > m_NumberOfTimeSteps ) {
		if( m_ChunkProperties.size() % m_NumberOfTimeSteps ) {
			LOG( Runtime, warning ) << "Cannot filter the metadata for each timestep. Your image contains of "
									<< m_ChunkProperties.size() << " chunks and " << m_NumberOfTimeSteps
									<< " timesteps. The number of chunks should be a multiple of number of timesteps!";
			return false;
		} else {
			size_t factor = m_ChunkProperties.size() / m_NumberOfTimeSteps;

			for ( size_t t = 0; t < m_ChunkProperties.size(); t += factor ) {
				m_TimeStepProperties.push_back( m_ChunkProperties[t] );
			}

			if( m_TimeStepProperties.size() != m_NumberOfTimeSteps ) {
				LOG(  Runtime, warning ) << "Something went wrong while filtering the properties of each timestep. We got "
										 << m_TimeStepProperties.size() << " timestep properties for " << m_NumberOfTimeSteps << " timestep.";
				return false;
			}
		}
	}

	return true;
}
bool ImageHolder::setImage( data::Image image )
{

	//we convert the image to an uint8_t data type
	typedef uint8_t TYPE;

	//some checks
	if( image.isEmpty() ) {
		LOG( Runtime, error ) << "Getting an empty image? Obviously something went wrong.";
		return false;
	}

	//check if the TYPE corresponds to the typeID of the image
	m_Image = image;

	// get some image information
	m_MinMax = image.getMinMax();
	m_ImageSize = image.getSizeAsVector();
	m_NumberOfTimeSteps = m_ImageSize[3];
	LOG( Debug, verbose_info )  << "Fetched image of size " << m_ImageSize << " and type "
								<< image.getMajorTypeName() << ".";

	//copy the image into continuous memory space and assure consistent data type
	data::ValuePtr<TYPE> imagePtr( ( TYPE * ) calloc( image.getVolume(), sizeof( TYPE ) ), image.getVolume() );
	LOG( Debug, verbose_info ) << "Needed memory: " << image.getVolume() * sizeof( TYPE ) / ( 1024.0 * 1024.0 ) << " mb.";
	image.copyToMem<TYPE>( &imagePtr[0] );
	LOG( Debug, verbose_info ) << "Copied image to continuous memory space.";

	//splice the image in its volumes -> we get a vector of t volumes
	if( m_NumberOfTimeSteps > 1 ) { //splicing is only necessary if we got more than 1 timestep
		m_ImageVector = imagePtr.splice( m_ImageSize[0] * m_ImageSize[1] * m_ImageSize[2] );
	} else {
		m_ImageVector.push_back( imagePtr );
	}

	LOG_IF( m_ImageVector.empty(), Runtime, error ) << "Size of image vector is 0!";

	if( m_ImageVector.size() != m_NumberOfTimeSteps ) {
		LOG( Runtime, error ) << "The number of timesteps (" << m_NumberOfTimeSteps
							  << ") does not coincide with the number of volumes ("  << m_ImageVector.size() << ").";
		return false;
	}

	LOG( Debug, verbose_info ) << "Spliced image to " << m_ImageVector.size() << " volumes.";

	//copy all the relevant meta information
	m_PropMap = static_cast<util::PropertyMap>( image );

	//workaround cause compiles do not understand to use getChunksAsVector directly in BOOST_FOREACH
	ChunkVector chVec = image.getChunksAsVector();
	BOOST_FOREACH( ChunkVector::const_reference chRef, chVec ) {
		m_ChunkProperties.push_back( static_cast<util::PropertyMap>( *chRef ) );
	}
	LOG( Debug, verbose_info ) << "Fetched " << m_ChunkProperties.size() << " chunk properties.";
	//image seems to be ok...i guess
	return filterRelevantMetaInformation(); //only return true if filtering was successfully
}

}
} //end namespace

