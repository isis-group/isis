// kate: show-indent on; indent-mode tab; indent-width 4; tab-width 4; replace-tabs off; auto-insert-doxygen on

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
#include <boost/foreach.hpp>
#include "CoreUtils/property.hpp"

namespace isis{ namespace data{

namespace _internal{
	
bool image_chunk_order::operator() ( const data::Chunk& a, const data::Chunk& b )const
{
	//@todo exception ??
	LOG_IF(not a.hasProperty("indexOrigin"),DataDebug,util::error)
		<< "The chunk has no position, it can not be sorted into the image.";
	LOG_IF(not a.hasProperty("acquisitionNumber"),DataDebug,util::warning)
		<< "The chunk has no acquisitionNumber, it may not be sorted into the image.";

	const util::fvector4 &posA=a.getProperty<util::fvector4>("indexOrigin");
	const util::fvector4 &posB=b.getProperty<util::fvector4>("indexOrigin");

	if(posA.lexical_less_reverse(posB)){ //if chunk is "under" the other - put it there
		LOG(DataDebug,util::verbose_info)
		<< "Successfully sorted chunks by position"
		<< " ("<< posA << " below " << posB << ")";
		return true;
	} 
	if(posA==posB) { //if the chunks have the same position, check if they can be sorted by time
		if(a.hasProperty("acquisitionTime") and b.hasProperty("acquisitionTime")){
			const float aTime=a.getProperty<u_int32_t>("acquisitionTime");
			const float bTime=b.getProperty<u_int32_t>("acquisitionTime");
			if(aTime< bTime){
				LOG(DataDebug,util::verbose_info)
				<< "Fallback sorted chunks by time"
				<< " ("<< aTime << " before " << bTime << ")";
				return true;
			}else if(bTime<aTime){
				return false;
			}
		}
		//if acquisitionTime is equal as well (or missing) fall back to acquisitionNumber
		const u_int32_t aNumber=a.getProperty<u_int32_t>("acquisitionNumber");
		const u_int32_t bNumber=b.getProperty<u_int32_t>("acquisitionNumber");
		if(aNumber<bNumber){
			//if they at least have different acquisitionNumber
			LOG(DataDebug,util::verbose_info)
			<< "Fallback sorted chunks by acquisition order"
			<< " ("<< aNumber << " before " << aNumber << ")";
			return true;
		}
		LOG_IF(aNumber==bNumber,DataDebug,util::error)<<"The Chunks cannot be sorted, won't insert";
	}
	return false;
}

}
	
Image::Image (_internal::image_chunk_order lt ) :set ( lt ),clean(true)
{
	addNeededFromString(needed);
	const size_t idx[]={0,0,0,0};
	init(idx);
}


bool Image::insertChunk ( const Chunk &chunk )
{
	if(not set.empty()){
		//if our first chunk and the incoming chunk do have different size, skip it
		if(set.begin()->sizeToVector() != chunk.sizeToVector()){
			LOG(DataDebug,util::info)
				<< "Ignoring chunk with different size. (" << chunk.sizeToString() << "!=" << set.begin()->sizeToString() << ")";
			return false;
		}

		//if our first chunk and the incoming chunk do have sequenceNumber and it differs, skip it
		const util::PropertyValue baseSeq=set.begin()->getPropertyValue("sequenceNumber");
		const util::PropertyValue insSeq=chunk.getPropertyValue("sequenceNumber");
		if(not (baseSeq.empty() or insSeq.empty() or (baseSeq==insSeq))){ 
			LOG(DataDebug,util::info)
				<< "Ignoring chunk because its sequenceNumber doesn't fit (" << insSeq->toString(false) << "!=" << baseSeq->toString(false) << ")";
			return false;
		}
	}
	if(not chunk.valid()){
		const util::PropMap::key_list missing=chunk.missing();
		LOG(DataLog,util::error)
			<< "Cannot insert chunk. Missing properties: " << util::list2string(missing.begin(),missing.end());
		return false;
	}
	if(set.insert(chunk).second){
		clean=false;
		return true;
	} else return false;
}


bool Image::reIndex() {
	if(set.empty()){
		clean=true;
		LOG(DataLog,util::warning) << "Reindexing an empty image is useless.";
		return true;
	}
	
	//redo lookup table
	lookup.resize(set.size());
	size_t idx=0;
	for(std::set<Chunk,_internal::image_chunk_order>::iterator it=set.begin();it!=set.end();it++,idx++)
		lookup[idx]=it;
	
	//get primary attributes from first chunk
	const unsigned short chunk_dims=chunksBegin()->dimRange().second+1;
	chunkVolume = chunksBegin()->volume();

	//copy sizes of the chunks to to the first chunk_dims sizes of the image
	size_t size[Chunk::n_dims];
	for(unsigned short i=0;i<chunk_dims;i++)
		size[i]=chunksBegin()->dimSize(i);

	//if there are many chunks, they must leave at least on dimension to the image to sort them in
	if(chunk_dims>=Image::n_dims){
		if(lookup.size()>1){
			LOG(DataLog,util::error)
			<< "Cannot handle multiple Chunks, if they have more than "
			<< Chunk::n_dims-1 << " dimensions";
			return false;
		}
		//if there is only one chunk, its ok - the image will consist only of this one, 
		//and commonGet will allways return <0,set.begin()->dim2Index()>
		//thus in this case voxel() equals set.begin()->voxel()
	} else {// OK there is at least one dimension to sort in the chunks 
		// check the chunks for at least one dimensional break - use that for the size of that dimension
		size[chunk_dims]= getChunkStride() ?:1;
		for(unsigned short i=chunk_dims+1;i<Image::n_dims;i++)//if there are dimensions left figure out their size
			size[i]= getChunkStride(size[i-1])/size[i-1] ?:1;
	}

	//get indexOrigin from the geometrically first chunk
	setPropertyValue("indexOrigin",chunksBegin()->getPropertyValue("indexOrigin"));

	//Clean up the properties 
	//@todo might fail if the image contains a prop that differs to that in the Chunks (which is equal in the chunks)
	util::PropMap common;
	std::set<std::string> uniques;
	set.begin()->toCommonUnique(common,uniques,true);
	
	for(ChunkIterator i= ++chunksBegin();i!=chunksEnd();i++){
		i->toCommonUnique(common,uniques,false);
	}
	LOG(DataDebug,util::info) << uniques.size() << " Chunk-unique properties found in the Image";
	LOG(DataDebug,util::verbose_info) << util::list2string(uniques.begin(),uniques.end(),", ");

	join(common);
	LOG(DataDebug,util::verbose_info) << "common properties saved into the image" << common;

	//remove common props from the chunks
	for(size_t i=0;i!=lookup.size();i++)
		getChunkAt(i).remove(common);
	LOG(DataDebug,util::info) << "common properties removed from " << set.size() << " chunks: " << common;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	//reconstruct some redundant information, if its missing
	//////////////////////////////////////////////////////////////////////////////////////////////////
		
	//if we have at least two slides (and have slides at all)
	if(chunk_dims==2 and size[2]>1){

		const util::fvector4 thisV=lookup[0]->getProperty<util::fvector4>("indexOrigin");
		const util::fvector4 nextV=lookup[1]->getProperty<util::fvector4>("indexOrigin");
		const util::fvector4 lastV=lookup[size[2]-1]->getProperty<util::fvector4>("indexOrigin");

		//check the slice vector
		const util::fvector4 distVecNorm= (lastV-thisV).norm();
		if(hasProperty("sliceVec")){
			const util::fvector4 sliceVec=getProperty<util::fvector4>("sliceVec");
			LOG_IF(distVecNorm!=sliceVec,DataLog,util::warning)
				<< "The existing sliceVec " << sliceVec
				<< " differs from the distance vector between chunk 0 and " << size[2]-1
				<< " " << distVecNorm;
		} else {
			LOG(DataDebug,util::info)
				<< "used the distance between chunk 0 and "	<< size[2]-1
				<< " to synthesize the missing sliceVec as " << distVecNorm;
			setProperty("sliceVec",distVecNorm);
		}

		//check the slice distance
		const float sliceDist=(nextV-thisV).len();
		util::fvector4 &voxelGap=getPropertyValue("voxelGap")->cast_to_Type<util::fvector4>();
		if(voxelGap[2]!=-std::numeric_limits<float>::infinity()){
			LOG_IF(voxelGap[2]!=sliceDist,DataLog,util::warning)
				<< "The existing slice distance (voxelGap[2]) " << voxelGap[2]
				<< " differs from the distance between chunk 0 and 1 " << sliceDist;
		} else {
			voxelGap[2]=sliceDist;
			LOG(DataDebug,util::info)
				<< "used the distance between chunk 0 and 1 to synthesize the missing slice distance (voxelGap[2]) as "
				<< sliceDist;
		}
	}
	//if we have read- and phase- vector
	if(hasProperty("readVec") and hasProperty("phaseVec")){
		util::fvector4 &read=operator[]("readVec")->cast_to_Type<util::fvector4>();
		util::fvector4 &phase=operator[]("phaseVec")->cast_to_Type<util::fvector4>();
		read.norm();
		phase.norm();
		const util::fvector4 crossVec =util::fvector4( //we could use their cross-product as sliceVector
			read[1]*phase[2]-read[2]*phase[1],
			read[2]*phase[0]-read[0]*phase[2],
			read[0]*phase[1]-read[1]*phase[0]
		);
		if(hasProperty("sliceVec")){
			const util::fvector4 sliceVec=getProperty<util::fvector4>("sliceVec");
			LOG_IF(crossVec!=sliceVec,DataLog,util::warning)
				<< "The existing sliceVec " << sliceVec
				<< " differs from the cross product of the read- and phase vector " << crossVec;
		} else {
			LOG(DataDebug,util::info)
				<< "used the cross product between readVec and phaseVec as sliceVec"
				<< getPropertyValue("sliceVec");
			LOG_IF(read.dot(phase)>0.01,DataLog,util::warning)
				<< "The cosine between the columns and the rows of the image is bigger than 0.01";
			setProperty("sliceVec",crossVec);
		}
	}
	
	init(size);
	clean=true;
	return true;
}

const Chunk& Image::getChunkAt(size_t at)const
{
	return *(lookup[at]);
}
Chunk& Image::getChunkAt(size_t at)
{
	//@todo we must cast away the const here because std::set has no non-const iterators
	Chunk &ret=const_cast<Chunk&>(*(lookup[at]));
	return ret;
}
	
Chunk& Image::getChunk (size_t first,size_t second,size_t third,size_t fourth) {
	if(not clean){
		LOG(DataDebug,util::info)
		<< "Image is not clean. Running reIndex ...";
		reIndex();
	}
	const size_t index=commonGet(first,second,third,fourth).first;
	return getChunkAt(index);
}
	
const Chunk& Image::getChunk (size_t first,size_t second,size_t third,size_t fourth) const {
	const size_t index=commonGet(first,second,third,fourth).first;
	return getChunkAt(index);
}

size_t Image::getChunkStride ( size_t base_stride )
{
	size_t ret;
	if(set.empty()){
		LOG(DataLog,util::error)
		<< "Trying to get chunk stride in an empty image";
		return 0;
	} else if (lookup.empty()) {
		LOG(DataDebug,util::error)
		<< "Lookup table for chunks is empty. Do reIndex() first!";
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
		const util::fvector4 thisV=lookup[0]->getProperty<util::fvector4>("indexOrigin");
		const util::fvector4 nextV=lookup[base_stride]->getProperty<util::fvector4>("indexOrigin");
		const util::fvector4 dist1 =nextV-thisV;
		
		// compare every follwing distance to that
		for(size_t i=base_stride;i<lookup.size()-base_stride;i+=base_stride){
			const util::fvector4 thisV=lookup[i]->getProperty<util::fvector4>("indexOrigin");
			const util::fvector4 nextV=lookup[i+base_stride]->getProperty<util::fvector4>("indexOrigin");
			const util::fvector4 dist =nextV-thisV;

			LOG(DataDebug,util::verbose_info)
			<< "Distance between chunk " << util::MSubject(i) << " and " << util::MSubject(i+base_stride)
			<< " is " << dist.len();
			
			if(dist.sqlen() > dist1.sqlen()*4){// found an dimensional break - leave
				ret=i+1;
				LOG(DataDebug,util::info)
				<< "Distance between chunk " << util::MSubject(i) << " and " << util::MSubject(i+base_stride)
				<< " is more then twice the first distance, assuming dimensional break at " << ret;
				return ret; 
			}
		}
	}
	//we didn't find any break, so we assume its a linear image |c c ... c|
	ret=lookup.size();
	LOG(DataDebug,util::info)
	<< "No dimensional break found, assuming it to be at the end (" << ret << ")";
	return ret;
}

std::list<util::PropMap::mapped_type> Image::getChunksProperties(const util::PropMap::key_type& key, bool unique)const
{
	std::list<util::PropertyValue > ret;
	for(ChunkSet::const_iterator i=set.begin();i!=set.end();i++){
		const util::PropertyValue &prop=i->getPropertyValue(key);
		if(unique && prop.empty()) //if unique is requested and the property is empty
			continue; //skip it
		else if(unique && !(ret.empty() ||  prop == ret.back()))
			//if unique is requested and the property is equal to the one added before
			continue;//skip it
		else
			ret.push_back(prop);
	}
	return ret;
}


Image::ChunkIterator Image::chunksBegin(){return set.begin();}
Image::ChunkIterator Image::chunksEnd(){return set.end();}


util::fvector4 Image::getFOVVec()const
{
	return fieldOfViewVec;
}

template <typename T> T Image::minValueInImage()const
{
	return 0;
}

template <typename T> T Image::maxValueInImage()const
{
	return 0;
}


ImageList::ImageList(ChunkList src)
{
	while(!src.empty()){
		value_type buff(new Image);
		for(ChunkList::iterator i=src.begin();i!=src.end();){
			if(not i->valid()){
				const util::PropMap::key_list missing=i->missing();
				LOG(DataLog,util::error)
					<< "Ignoring invalid chunk. Missing properties: " << util::list2string(missing.begin(),missing.end());
				src.erase(i++);
			} else {
				if(buff->insertChunk(*i))
					src.erase(i++);
				else
					i++;
			}
		}
		if(buff->chunksBegin()!=buff->chunksEnd()){
			buff->reIndex();
			if(buff->valid())
				push_back(buff);
			else {
				const util::PropMap::key_list missing=buff->missing();
				LOG(DataLog,util::error)
					<< "Cannot insert image. Missing properties: " << util::list2string(missing.begin(),missing.end());
			}
		}
	}
}


}}
