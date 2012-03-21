#include "filter.hpp"

namespace isis
{
namespace filter
{
namespace _internal
{

bool _internal::FilterBase::run( boost::shared_ptr< util::ProgressFeedback > )
{
	if( isValid() ) {
		return process();
	} else {
		LOG( data::Runtime, warning ) << "The filter \"" << getFilterName()
									  << "\" is not valid. Will not run it!";
		return false;
	}
}

}
}
}
