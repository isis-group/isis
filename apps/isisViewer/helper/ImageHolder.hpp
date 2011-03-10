#ifndef IMAGEHOLDER_HPP
#define IMAGEHOLDER_HPP

#include <boost/foreach.hpp>
#include <vector>
#include <CoreUtils/propmap.hpp>
#include <DataStorage/image.hpp>
#include "common.hpp"

namespace isis {
namespace viewer {
	
/**
 * Class that holds one image in a vector of data::ValuePtr's
 * It ensures the data is hold in continuous memory and only consists of one type.
 * Furthermore this class handles the meta information of the image
 */

class ImageHolder
{
	
public:
	typedef data::_internal::ValuePtrBase::Reference ImagePointerType;
	
	ImageHolder();
	
	template<class TYPE>
	bool setImage( data::Image image ) 
	{
		//TODO debug
		m_DebugImage = image;
	
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
		LOG( Debug, verbose_info ) << "Needed memory: " << image.getVolume() << " * " << sizeof(TYPE) << " = " 
			<< image.getVolume() * sizeof(TYPE) << ".";
		image.copyToMem<TYPE>( &imagePtr[0] );
		LOG( Debug, verbose_info) << "Copied image to continuous memory space.";
		//splice the image in its volumes -> we get a vector of t volumes
		if(m_NumberOfTimeSteps > 1) //splicing is only necessary if we got more than 1 timestep
		{
			m_ImageVector = imagePtr.splice( m_ImageSize[0] * m_ImageSize[1] * m_ImageSize[2] );
		} else {
			m_ImageVector.push_back( imagePtr );
		}
			
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
		return filterRelevantMetaInformation(); //only return true if filtering was successfully
	}

		
	std::vector< ImagePointerType >
		getImageVector() const { return m_ImageVector; }
	std::vector< util::PropertyMap > 
		getChunkProperties() const { return m_ChunkProperties; }
	std::vector< util::PropertyMap >
		getTimeStepProperties() const { return m_TimeStepProperties; }
	util::PropertyMap 
		getPropMap() const { return m_PropMap; }
	util::FixedVector<size_t, 4> 
		getImageSize() const { return m_ImageSize; }
	//TODO debug
	data::Image m_DebugImage;
	
private:
	size_t m_NumberOfTimeSteps;
	unsigned short m_TypeID;
	util::FixedVector<size_t, 4> m_ImageSize;
	util::PropertyMap m_PropMap;
	std::vector< util::PropertyMap > m_ChunkProperties;
	std::vector< util::PropertyMap > m_TimeStepProperties;
	
	std::vector< ImagePointerType > m_ImageVector;
	
	
	bool filterRelevantMetaInformation();
	
};

}} //end namespace

#endif