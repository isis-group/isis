#ifndef ISIS_FILTER_GAUSSIAN
#define ISIS_FILTER_GAUSSIAN

#include "DataStorage/filter.hpp"
#include "GaussianKernelFilter.hpp"
#include "ConvolutionFilter.hpp"

namespace isis
{
namespace filter
{

class GaussianFilter : public ImageFilterInPlace
{
	typedef float ValueType;
public:
	std::string getFilterName() const { return std::string( "GaussianFilter" ); }
	std::string getDescription() const { return std::string( "Inplace filter that performs a gaussian kernel to an image." ); }
	bool isValid() const {
		return  parameterMap.hasProperty( "sigma" );
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