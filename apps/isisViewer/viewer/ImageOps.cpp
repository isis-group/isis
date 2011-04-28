#include "ImageOps.hpp"

namespace isis {
namespace viewer {

std::list< util::ivector4 > ImageOps::getPositionsWithValue(double value, const data::Image &image) 
{
	std::list<util::ivector4> retList;
	data::ValuePtr<ImageOps::TYPE> imagePtr( ( ImageOps::TYPE * ) calloc( image.getVolume(), sizeof( ImageOps::TYPE ) ), image.getVolume() );
	image.copyToMem<ImageOps::TYPE>( &imagePtr[0] );
	std::list<size_t> posList;
	for(size_t i = 0; i< image.getVolume();i++)
	{
		if(imagePtr[i] == value ) {
			posList.push_back(i);
		}
	}
	size_t tmpPos[4];
	BOOST_FOREACH( std::list<size_t>::const_reference pos, posList) 
	{
		image.getCoordsFromLinIndex( pos, tmpPos );
		retList.push_front( util::ivector4( tmpPos[0], tmpPos[1], tmpPos[2], tmpPos[3] ) );
		
	}
	return retList;
}


}}