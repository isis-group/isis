#ifndef ISIS_FILTER_GAUSSIANKERNEL_HPP
#define ISIS_FILTER_GAUSSIANKERNEL_HPP

#include "DataStorage/filter.hpp"

namespace isis
{
namespace filter
{

class GaussianKernelFilter : public _internal::ChunkOutputFilter
{
	typedef float ValueType;
public:
	std::string getFilterName() const { return std::string( "GaussianKernelFilter" ); }
	std::string getDescription() const { return std::string( "Output filter that calculates a gaussian kernel." ); }
	bool isValid() const {
		return !parameters["sigma"].isEmpty();
	}
	bool process();
private:
	ValueType xxgauss( const ValueType &x, const ValueType &sigma );
};
}
}

#endif