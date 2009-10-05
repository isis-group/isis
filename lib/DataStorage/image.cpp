//
// C++ Implementation: image
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "image.hpp"

namespace isis{ namespace data{
	
Image::Image (_internal::image_lt lt ) :
std::set< isis::data::_internal::ChunkReference, isis::data::_internal::image_lt > ( lt )
{
	const size_t idx[]={0,0,0,0};
	init(idx);
}

}}