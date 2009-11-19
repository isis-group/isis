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
	MAKE_LOG(DataLog);
	if(set.empty()){
		clean=true;
		LOG(DataLog,util::info) << "Reindexing an empty image." << std::endl;
		return true;
	}
	
	//redo lookup table
	lookup.resize(set.size());
	size_t idx=0;
	for(std::set<Chunk,_internal::image_chunk_order>::iterator it=set.begin();it!=set.end();it++,idx++)
		lookup[idx]=it;
	
	//get primary attributes from first chunk
	const unsigned short chunk_dims=chunksBegin()->dimRange().second;
	chunkVolume = chunksBegin()->volume();

	//copy sizes of the chunks to to the first chunk_dims sizes of the image
	size_t size[Chunk::n_dims];
	for(unsigned short i=0;i<chunk_dims;i++)
		size[i]=chunksBegin()->dimSize(i);

	//if there are many chunks, they must leave at least on dimension to the image to sort them in
	if(chunk_dims>=Chunk::n_dims){
		if(lookup.size()>1){
			LOG(DataLog,util::error)
			<< "Cannot handle multiple Chunks, if they have more than "
			<< Chunk::n_dims-1 << " dimensions" << std::endl;
			return false;
		}
		//if there is only one chunk, its ok - the image will consist only of this one, 
		//and commonGet will allways return <0,set.begin()->dim2Index()>
		//thus voxel() equals set.begin()->voxel()
	} else {// OK there is at least one dimension to sort in the chunks 
		// check the chunks for at least one dimensional break - use that for the size of that dimension
		size[chunk_dims]= getChunkStride() ?:1;
		for(unsigned short i=chunk_dims+1;i<Chunk::n_dims;i++)//if there are dimensions left figure out their size
			size[i]= getChunkStride(size[i-1])/size[i-1] ?:1;
	}
	
	
	init(size);
	clean=true;
	return true;
}

std::pair<size_t,size_t> Image::commonGet (size_t first,size_t second,size_t third,size_t fourth) const
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
	
	const size_t idx[]={first,second,third,fourth};
	if(!rangeCheck(idx)){
		LOG(DataDebug,isis::util::error)
		<< "Index " << first << "|" << second << "|" << third << "|" << fourth << " is out of range (" << sizeToString() << ")"
		<< std::endl;
	}
	
	const size_t index=dim2Index(idx);
	return std::make_pair(index/chunkVolume,index%chunkVolume);
}
	
const Chunk& Image::getChunkAt(size_t at)const
{
	return *(lookup[at]);
}
Chunk& Image::getChunkAt(size_t at)
{
	//we must cast away the const here because std::set has no non-const iterators
	Chunk &ret=const_cast<Chunk&>(*(lookup[at]));
	return ret;
}
	
Chunk& Image::getChunk (size_t first,size_t second,size_t third,size_t fourth) {
	MAKE_LOG(DataDebug);
	if(not clean){
		LOG(DataDebug,util::info)
		<< "Image is not clean. Running reIndex ..." << std::endl;
		reIndex();
	}
	const size_t index=commonGet(first,second,third,fourth).first;
	return getChunkAt(index);
}
	
const Chunk& Image::getChunk (size_t first,size_t second,size_t third,size_t fourth) const {
	const size_t index=commonGet(first,second,third,fourth).first;
	return getChunkAt(index);
}

size_t Image::getChunkStride ( size_t base_stride ) {
	MAKE_LOG(DataLog);
	MAKE_LOG(DataDebug);
	size_t ret;
	if(set.empty()){
		LOG(DataLog,util::error)
		<< "Trying to get chunk stride in an empty image" << std::endl;
		return 0;
	} else if (lookup.empty()) {
		LOG(DataDebug,util::error)
		<< "Lookup table for chunks is empty. Do reIndex() first!" << std::endl;
		return 0;
	} else if(lookup.size()>3*base_stride) {
		/* there can't be any stride with less than 3*base_stride chunks (which would actually be an invalid image)
		 * _____
		 * |c c| has no stride/dimensional break
		 * _____
		 * |c c|
		 * |c  | has a dimensional break, but is invalid
		 * _____
		 * |c c|
		 * |c c| is the first reasonable case
		 */
		
		// get the distance between first and second chunk for comparision
		util::fvector4 thisV=lookup[0]->getProperty<util::fvector4>("indexOrigin");
		util::fvector4 nextV=lookup[base_stride]->getProperty<util::fvector4>("indexOrigin");
		const util::fvector4 dist1 =nextV-thisV;
		
		// compare every follwing distance to that
		for(size_t i=base_stride;i<lookup.size()-base_stride;i+=base_stride){
			util::fvector4 thisV=lookup[i]->getProperty<util::fvector4>("indexOrigin");
			util::fvector4 nextV=lookup[i+base_stride]->getProperty<util::fvector4>("indexOrigin");
			const util::fvector4 dist =nextV-thisV;

			LOG(DataDebug,util::verbose_info)
			<< "Distance between chunk " << util::MSubject(i) << " and " << util::MSubject(i+base_stride)
			<< " is " << dist.len() << std::endl;
			
			if(dist.sqlen() > dist1.sqlen()*4){// found an dimensional break - leave
				ret=i+1;
				LOG(DataDebug,util::info)
				<< "Distance between chunk " << util::MSubject(i) << " and " << util::MSubject(i+base_stride)
				<< " is more then twice the first distance, assuming dimensional break at " << ret << std::endl;
				return ret; 
			}
		}
	}
	//we didn't find any break, so we asumme its a linear image |c c ... c|
	ret=lookup.size();
	LOG(DataDebug,util::info)
	<< "No dimensional break found, assuming it to be at the end (" << ret << ")" << std::endl;
	return ret;
}

Image::ChunkIterator Image::chunksBegin(){return set.begin();}
Image::ChunkIterator Image::chunksEnd(){return set.end();}

}}