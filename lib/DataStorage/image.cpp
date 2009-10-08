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
#include "CoreUtils/vector.hpp"

namespace isis{ namespace data{

namespace _internal{
	
bool image_chunk_order::operator() ( const isis::data::_internal::ChunkReference& a, const isis::data::_internal::ChunkReference& b )
{
	MAKE_LOG(DataDebug);
	const PropertyObject &propA=*a;
	const PropertyObject &propB=*b;

	//@todo exception ??
	if(!(propA.hasProperty("position") && propB.hasProperty("position"))){
		LOG(DataDebug,isis::util::error) << "The chunk has no position, it can not be sorted" << std::endl;
		return false;
	}

	const isis::util::fvector4 posA=propA.getProperty<isis::util::fvector4>("position");
	const isis::util::fvector4 posB=propA.getProperty<isis::util::fvector4>("position");
	
	return posA<posB;
}

}
	
Image::Image (_internal::image_chunk_order lt ) :
std::set< isis::data::_internal::ChunkReference, isis::data::_internal::image_chunk_order > ( lt ),
PropertyObject(needed)
{
	const size_t idx[]={0,0,0,0};
	init(idx);
}


bool Image::insertChunk ( const isis::data::_internal::ChunkReference &chunk ) {
	insert(chunk);
}


}}