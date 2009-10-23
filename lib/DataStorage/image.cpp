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
	
bool image_chunk_order::operator() ( const data::Chunk& a, const data::Chunk& b )const
{
	MAKE_LOG(DataDebug);

	//@todo exception ??
	if(!(a.hasProperty("indexOrigin") && a.hasProperty("indexOrigin"))){
		LOG(DataDebug,util::error) << "The chunk has no position, it can not be sorted into the image." << std::endl;
		return false;
	}

	const util::fvector4 &posA=a.getProperty<util::fvector4>("indexOrigin");
	const util::fvector4 &posB=b.getProperty<util::fvector4>("indexOrigin");
	
	return posA.lexical_less_reverse(posB);
}

}
	
Image::Image (_internal::image_chunk_order lt ) :set ( lt ),PropertyObject(needed),clean(true)
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
		LOG(DataLog,util::error)
			<< "Cannot insert chunk. Missing properties: " << util::list2string(missing.begin(),missing.end(),", ") << std::endl;
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
	if(set.empty()){
		clean=true;
		LOG(DataLog,util::info) << "Reindexing an empty image." << std::endl;
		return true;
	}
	size_t idx=0;
	for(std::set<Chunk,_internal::image_chunk_order>::iterator it=set.begin();it!=set.end();it++,idx++)
		lookup[idx]=it;
	
	const unsigned short chunk_dims=chunksBegin()->dimRange().second;

	size_t size[Chunk::n_dims];
	for(unsigned short i=0;i<chunk_dims;i++)
		size[i]=chunksBegin()->dimSize(i);

	if(chunk_dims>=Chunk::n_dims){
		if(lookup.size()>1){
			LOG(DataLog,util::error)
			<< "Cannot handle multiple Chunks, if they have more than "
			<< Chunk::n_dims-1 << " dimensions" << std::endl;
			return false;
		}
	} else {
		size[chunk_dims]= getChunkStride() ?:1;
		for(unsigned short i=chunk_dims+1;i<Chunk::n_dims;i++)
			size[i]= getChunkStride(size[i-1])/size[i-1] ?:1;
	}
	init(size);
	clean=true;
	return true;
}

std::pair<size_t,size_t>
Image::commonGet ( const size_t& first, const size_t& second, const size_t& third, const size_t& fourth ) const
{
	MAKE_LOG(DataDebug);
	if(not clean){
		LOG(DataDebug,util::error)
		<< "Getting data from a non indexed image will result in undefined behavior. Run reIndex first." << std::endl;
	}
	if(set.empty()){
		LOG(DataDebug,util::error)
		<< "Getting data from a empty image will result in undefined behavior." << std::endl;
	}
	
	const size_t dim[]={first,second,third,fourth};
	const size_t chunkStride=set.begin()->volume();
	const size_t index=dim2Index(dim);
	return std::make_pair(index,chunkStride);
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
	const std::pair<size_t,size_t> index=commonGet(first,second,third,fourth);
	return *(lookup[index.first/index.second]);
}

size_t Image::getChunkStride ( size_t base_stride ) {
	MAKE_LOG(DataLog);
	MAKE_LOG(DataDebug);
	size_t ret;
	if(lookup.empty()){
		LOG(DataLog,util::error)
		<< "Trying to get chunk stride in an empty image" << std::endl;
		return 0;
	} else if(lookup.size()>3*base_stride) {
		/// there cant be any stride with less than 3*base_stride chunks in (which would actually be an invalid image)
		util::fvector4 thisV=lookup[0]->getProperty<util::fvector4>("indexOrigin");
		util::fvector4 nextV=lookup[base_stride]->getProperty<util::fvector4>("indexOrigin");
		const util::fvector4 dist1 =nextV-thisV;
		for(size_t i=base_stride;i<lookup.size()-base_stride;i+=base_stride){
			util::fvector4 thisV=lookup[i]->getProperty<util::fvector4>("indexOrigin");
			util::fvector4 nextV=lookup[i+base_stride]->getProperty<util::fvector4>("indexOrigin");
			const util::fvector4 dist =nextV-thisV;

			LOG(DataDebug,util::verbose_info)
			<< "Distance between chunk " << util::MSubject(i) << " and " << util::MSubject(i+base_stride)
			<< " is " << dist.len() << std::endl;
			if(dist.sqlen() > dist1.sqlen()*4){
				ret=i+1;
				LOG(DataDebug,util::info)
				<< "Distance between chunk " << util::MSubject(i) << " and " << util::MSubject(i+base_stride)
				<< " is more then twice the first distance, assuming dimensional break at " << ret << std::endl;
				return ret;
			}
		}
	}
	ret=lookup.size();
	LOG(DataDebug,util::info)
	<< "No dimensional break found, assuming it to be at the end (" << ret << ")" << std::endl;
	return ret;
}

Image::ChunkIterator Image::chunksBegin(){return set.begin();}
Image::ChunkIterator Image::chunksEnd(){return set.end();}

}}