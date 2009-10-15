#include "chunk.hpp"
#include "image.hpp"

// Stuff needed for any Chunk
const isis::util::PropMap::key_type isis::data::_internal::ChunkBase::needed[]=
{
	"indexOrigin"
};

// Stuff needed for any Image
const isis::util::PropMap::key_type isis::data::Image::needed[]=
{
	"indexOrigin"
};

