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
	if(!chunk.sufficient()){
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
	for(std::set<Chunk,_internal::image_chunk_order>::iterator it=set.begin();it!=set.end();it++)
		lookup[idx]=it;
}


Chunk Image::getChunk ( const size_t& first, const size_t& second, const size_t& third, const size_t& fourth ) const {
	MAKE_LOG(DataLog);
	MAKE_LOG(DataDebug);
	MemChunk<int> dummy(0,0,0,0);
	if(not clean){
		LOG(DataDebug,util::error)
		<< "Cannot get data from a non indexed image. Run reindex first." << std::endl;
		return dummy;
	}
	const size_t dim[]={first,second,third,fourth};
	return *(lookup[dim2Index(dim)]);
/*	dummy.setProperty("indexOrigin",util::fvector4(first,second,third,fourth));
	iterator found=upper_bound(dummy);
	if(found != end() && (--found)!= end())
		return *found;
	else {
		LOG(DataLog,util::error)
		<< "No Chunk found for a voxel at " << util::MSubject(util::fvector4(first,second,third,fourth)) << std::endl;
		return dummy;
	}*/
}


}}