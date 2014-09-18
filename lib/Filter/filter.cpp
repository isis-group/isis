#include "filter.hpp"

namespace isis
{
namespace filter
{

namespace _internal
{

void FilterBase::setInput ( const std::string &label, const data::Image &image )
{
	m_additionalImages[label] = std::shared_ptr<data::Image> ( new data::Image( image ) );
}

void FilterBase::setInput ( const std::string &label, const data::Chunk &chunk )
{
	m_additionalChunks[label] = std::shared_ptr<data::Chunk> ( new data::Chunk( chunk ) );
}

void FilterBase::setParameters ( const util::ParameterMap &map )
{
	for( util::ParameterMap::const_reference mapElem :  map ) {
		parameters[mapElem.first] = mapElem.second;
	}
}


}
}
}
