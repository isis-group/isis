#include "chunk.hpp"
#include "image.hpp"

/*
 * Add needed properties for Property-objects.
 * Objects which lack any property will be rejected by the system.
 * see PropertyObject::sufficient()
 */

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

