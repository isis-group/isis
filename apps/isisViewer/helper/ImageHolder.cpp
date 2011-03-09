
#include "ImageHolder.hpp"

namespace isis {
namespace viewer {
	
ImageHolder::ImageHolder()
	: m_NumberOfTimeSteps(0)
{}



bool
ImageHolder::filterRelevantMetaInformation()
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
				LOG(  Runtime, warning ) << "Something went wrong while filtering the properties of each timestep. We got " 
					<< m_TimeStepProperties.size() << " timestep properties for " << m_NumberOfTimeSteps << " timestep.";
				return false;
			}				
		}
	} 
	return true;
}

}} //end namespace

