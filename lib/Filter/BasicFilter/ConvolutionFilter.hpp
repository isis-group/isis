#ifndef ISIS_FILTER_CONVOLUTION
#define ISIS_FILTER_CONVOLUTION

#include "DataStorage/filter.hpp"

namespace isis
{
namespace filter
{

class ConvolutionFilter : public ChunkFilterInPlace
{
	typedef float ValueType;
public:
	std::string getFilterName() const { return std::string( "ConvolveFilter" ); }
	std::string getDescription() const { return std::string( "Inplace filter that performs a convolution with a given kernel to a chunk." ); }
	bool isValid() const {
		return  parameterMap.hasProperty( "convolveRow" )
				&& parameterMap.hasProperty( "convolveColumn" )
				&& parameterMap.hasProperty( "convolveSlice" )
				&& m_kernel;
	}
	bool process( data::Chunk & );

	void setKernel( const data::Chunk &kernel );

private:
	void convolve( data::Chunk &chunk, const data::Chunk &kernel, data::dimensions dim );
	boost::shared_ptr<data::Chunk> m_kernel;

};


}
}

#endif