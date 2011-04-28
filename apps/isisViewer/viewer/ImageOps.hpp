#ifndef IMAGEOPS_HPP
#define IMAGEOPS_HPP

#include "common.hpp"
#include "ImageHolder.hpp"

namespace isis {
namespace viewer {

class ImageOps
{
public:
	typedef double TYPE;
	static std::list<util::ivector4> getPositionsWithValue( double value, const data::Image &image );
};
}}



#endif