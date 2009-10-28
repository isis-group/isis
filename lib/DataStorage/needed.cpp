#include "chunk.hpp"
#include "image.hpp"

// Stuff needed for every Chunk
const isis::util::PropMap::key_type isis::data::_internal::ChunkBase::needed[]=
{
	"indexOrigin",
	"acquisitionNumber"
};

// Stuff needed for any Image
const isis::util::PropMap::key_type isis::data::Image::needed[]=
{
	"voxelSize",
	"readVec",
	"phaseVec",
	"sliceVec"
};

