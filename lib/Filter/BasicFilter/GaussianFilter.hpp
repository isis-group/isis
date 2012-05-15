#ifndef ISIS_FILTER_GAUSSIAN
#define ISIS_FILTER_GAUSSIAN

#include "GaussianKernelFilter.hpp"
#include "ConvolutionFilter.hpp"
#include "DataStorage/filter.hpp"
// #include "../common.hpp"

#include <numeric>

namespace isis
{
namespace filter
{

class GaussianFilter : public _internal::ImageFilterInPlace
{
	typedef float ValueType;
public:
	GaussianFilter();

	std::string getFilterName() const { return std::string( "GaussianFilter" ); }
	std::string getDescription() const { return std::string( "Inplace filter that performs a gaussian kernel to an image." ); }

	bool isValid() const {
		return  ( !parameters["sigma"].isEmpty() && !std::isnan( parameters["sigma"].as<float>() ) )
				|| ( !parameters[ "fwhm" ].isEmpty() && !std::isnan( parameters["fwhm"].as<float>() ) ) ;
	}
	bool process( data::Image & );

private:
	void convolve( data::Chunk &chunk, const data::Chunk &kernel, data::dimensions dim );

	GaussianKernelFilter m_GaussianKernelFilter;
	ConvolutionFilter m_ConvolutionFilter;
};


}
}

#endif