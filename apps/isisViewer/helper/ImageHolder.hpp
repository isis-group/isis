#ifndef IMAGEHOLDER_HPP
#define IMAGEHOLDER_HPP

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <CoreUtils/propmap.hpp>
#include <DataStorage/image.hpp>
#include "common.hpp"

namespace isis {
namespace viewer {
	
/**
 * Class that holds one image in a vector of boost::shared_ptr.
 * It ensures the data is hold in continuous memory and one type.
 */

template <class TYPE>
class ImageHolder
{
	
public:
	ImageHolder();
	bool setImage( data::Image );
	
	std::vector< data::_internal::ValuePtrBase::Reference >
		getImageVector() const { return m_ImageVector; }
	std::vector< util::PropertyMap > 
		getChunkProperties() const { return m_ChunkProperties; }
	std::vector< util::PropertyMap >
		getTimeStepProperties() const { return m_TimeStepProperties; }
	util::PropertyMap 
		getPropMap() const { return m_PropMap; }
	
	
private:
	size_t m_NumberOfTimeSteps;
	bool m_IsValid;
	unsigned short m_TypeID;
	util::fvector4 m_ImageSize;
	util::PropertyMap m_PropMap;
	std::vector< util::PropertyMap > m_ChunkProperties;
	std::vector< util::PropertyMap > m_TimeStepProperties;
	
	// do not try this at home!!!
	std::vector< data::_internal::ValuePtrBase::Reference > m_ImageVector;
	
	bool filterRelevantMetaInformation();
	
};

}} //end namespace

#include "ImageHolder_impl.hpp"
#endif