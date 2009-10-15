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
	
bool image_chunk_order::operator() ( const isis::data::Chunk& a, const isis::data::Chunk& b )
{
	MAKE_LOG(DataDebug);

	//@todo exception ??
	if(!(a.hasProperty("indexOrigin") && a.hasProperty("indexOrigin"))){
		LOG(DataDebug,isis::util::error) << "The chunk has no position, it can not be sorted into the image." << std::endl;
		return false;
	}

	const isis::util::fvector4 &posA=a.getProperty<isis::util::fvector4>("indexOrigin");
	const isis::util::fvector4 &posB=b.getProperty<isis::util::fvector4>("indexOrigin");
	
	return posA<posB;
}

}
	
Image::Image (_internal::image_chunk_order lt ) :
std::set< Chunk, isis::data::_internal::image_chunk_order > ( lt ),
PropertyObject(needed)
{
	const size_t idx[]={0,0,0,0};
	init(idx);
}


bool Image::insertChunk ( const Chunk &chunk ) {
	MAKE_LOG(DataLog);
	if(!chunk.sufficient()){
		LOG(DataLog,isis::util::error)
			<< "Cannot insert insufficient chunk" << std::endl;
		return false;
	}
		
	return insert(chunk).second;
}


Chunk Image::getChunk ( const size_t& first, const size_t& second, const size_t& third, const size_t& fourth ) const {

}


}}