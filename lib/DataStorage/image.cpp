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
	
	return posA.lexical_less_reverse(posB);
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
		const util::PropMap::key_list missing=chunk.missing();
		LOG(DataLog,isis::util::error)
			<< "Cannot insert chunk. Missing proprties: " << util::list2string(missing.begin(),missing.end(),", ") << std::endl;
		return false;
	}
	if(set.insert(chunk).second){
		clean=false;
		return true;
	} else return false;
}


bool Image::reIndex() {
	//@todo for now this only works for 1D difference between Chunk and Image
	MAKE_LOG(DataLog);
	lookup.resize(set.size());
	if(lookup.size()==0){
		clean=true;
		LOG(DataLog,util::info) << "Reindexing an empty image." << std::endl;
		return true;
	}
	size_t idx=0;
	for(std::set<Chunk,_internal::image_chunk_order>::iterator it=set.begin();it!=set.end();it++,idx++)
		lookup[idx]=it;

	const ChunkIterator first=set.begin();

	unsigned short chunk_dims=0;
	size_t size[Chunk::n_dims];
	for(size_t i=0;i<Chunk::n_dims;i++){
		size[i]=first->size(i);
		if(size[i]>1)chunk_dims=i+1;
	}

	if(chunk_dims>=Chunk::n_dims){
		if(lookup.size()>1){
			LOG(DataLog,util::error)
			<< "Cannot handle multiple Chunks, if they have more than "
			<< Chunk::n_dims-1 << " dimensions" << std::endl;
			return false;
		}
	} else
		size[chunk_dims]=lookup.size();
	init(size);
	clean=true;
	return true;
}

Chunk Image::getChunk ( const size_t& first, const size_t& second, const size_t& third, const size_t& fourth ) {
	MAKE_LOG(DataDebug);
	if(not clean){
		LOG(DataDebug,util::info)
		<< "Image is not clean. Running reIndex ..." << std::endl;
		reIndex();
	}
	return const_cast<const Image*>(this)->getChunk(first,second,third,fourth);
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
	const size_t index=dim2Index(dim);
	return *(lookup[index/chunkStride]);
}


std::_Rb_tree_const_iterator< Chunk > Image::chunksBegin(){return set.begin();}
std::_Rb_tree_const_iterator< Chunk > Image::chunksEnd(){return set.end();}

}}