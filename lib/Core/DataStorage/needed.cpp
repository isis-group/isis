#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#include "DataStorage/chunk.hpp"
#include "DataStorage/image.hpp"

/*
 * Add needed properties for Property-objects.
 * Objects which lack any of these properties will be rejected by the system.
 * see PropertyObject::sufficient()
 */

// Stuff needed for every Chunk
const char *isis::data::_internal::ChunkBase::neededProperties = "\
	indexOrigin\
	acquisitionNumber\
	voxelSize\
	readVec\
	phaseVec\
";

// Stuff needed for any Image
const char *isis::data::Image::neededProperties = "\
	voxelSize\
	readVec\
	phaseVec\
	sliceVec\
";

