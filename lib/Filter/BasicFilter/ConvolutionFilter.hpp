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
				&& m_additionalChunks.find( "kernel" ) != m_additionalChunks.end();
	}
	bool process( data::Chunk & );

private:
	void convolve( data::Chunk &chunk, const data::Chunk &kernel, data::dimensions dim );

};


}
}

#endif