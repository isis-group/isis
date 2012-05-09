#include "filter.hpp"

namespace isis
{
namespace filter
{

namespace _internal
{
FilterBase::FilterBase()
	: m_inputIsSet( false )
{}

void FilterBase::setInput ( const std::string &label, const data::Image &image )
{
	m_additionalImages[label] = boost::shared_ptr<data::Image> ( new data::Image( image ) );
}

void FilterBase::setInput ( const std::string &label, const data::Chunk &chunk )
{
	m_additionalChunks[label] = boost::shared_ptr<data::Chunk> ( new data::Chunk( chunk ) );
}

}
}
}
