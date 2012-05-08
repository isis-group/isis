#include "filter.hpp"

namespace isis
{
namespace filter
{
namespace _internal
{
FilterBase::FilterBase()
	: m_inputIsSet(false)
{}

bool FilterBase::run( )
{
	if( isValid() ) {
		filterStartedSignal( getFilterName() );
		bool success = process();
		filterFinishedSignal( getFilterName(), success );
		return success;
	} else {
		LOG( data::Runtime, warning ) << "The filter \"" << getFilterName()
									  << "\" is not valid. Will not run it!";
		return false;
	}
}


}
}
}
