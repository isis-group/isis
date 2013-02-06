#ifndef ISIS_FILTER_FREQUENCY
#define ISIS_FILTER_FREQUENCY

#include "../filter.hpp"
#include "../common.hpp"

namespace isis
{
namespace filter
{
class FrequencyFilter : public _internal::ImageFilterInPlace
{
	typedef float ValueType;
public:
	FrequencyFilter();

	std::string getFilterName() const { return std::string( "FrequencyFilter" ); }
	std::string getDescription() const { return std::string( "Inplace filter that performs a frequncy filtering to a given image." ); }
	bool isValid() const {
		return true;
	}
	bool process( data::Image & );

};
}
}



#endif