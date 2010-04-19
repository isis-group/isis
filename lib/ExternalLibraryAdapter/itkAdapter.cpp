#include "itkAdapter.hpp"


namespace isis
{
namespace adapter
{

itkAdapter::itkAdapter( const boost::shared_ptr<isis::data::Image> src )
		: m_ImageISIS( src )
{}



}} //end namespace