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
	
bool image_chunk_order::operator() ( const isis::data::Chunk& a, const isis::data::Chunk& b )const
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
	
Image::Image (_internal::image_chunk_order lt ) :set ( lt ),PropertyObject(needed),clean(false)
{
	const size_t idx[]={0,0,0,0};
	init(idx);
}


bool Image::insertChunk ( const Chunk &chunk ) {
	MAKE_LOG(DataLog);
	if(not set.empty() && set.begin()->volume() != chunk.volume()){
		LOG(DataLog,util::error)
			<< "Cannot insert chunk, because its volume doesn't fit with the volume of the chunks allready in this image." << std::endl;
		return false;
	} 
	if(not chunk.sufficient()){
		LOG(DataLog,isis::util::error)
			<< "Cannot insert insufficient chunk" << std::endl;
		return false;
	}
	if(set.insert(chunk).second){
		clean=false;
		return true;
	} else return false;
}


bool Image::reIndex() {
	lookup.resize(set.size());
	size_t idx=0;
	for(std::set<Chunk,_internal::image_chunk_order>::iterator it=set.begin();it!=set.end();it++,idx++)
		lookup[idx]=it;
}


Chunk Image::getChunk ( const size_t& first, const size_t& second, const size_t& third, const size_t& fourth ) const {
	MAKE_LOG(DataDebug);
	if(not clean){
		LOG(DataDebug,util::error)
		<< "Getting data from a non indexed image will result undefined behavior. Run reIndex first." << std::endl;
	}
	if(set.empty()){
		LOG(DataDebug,util::error)
		<< "Getting data from a empty image will result undefined behavior." << std::endl;
	}
	
	const size_t dim[]={first,second,third,fourth};
	const size_t chunkStride=set.begin()->volume();
	return *(lookup[dim2Index(dim)/chunkStride]);
}


std::_Rb_tree_const_iterator< Chunk > Image::chunksBegin(){return set.begin();}
std::_Rb_tree_const_iterator< Chunk > Image::chunksEnd(){return set.end();}

}}