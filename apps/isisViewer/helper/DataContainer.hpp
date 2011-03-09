#ifndef DATACONTAINER_HPP
#define DATACONTAINER_HPP

#include <vector>
#include "ImageHolder.hpp"

namespace isis {
namespace viewer {
	
class DataContainer : public std::vector<ImageHolder>
{
public:
	bool addImage( const data::Image& );
		
	
};

	
}} // end namespace

#endif