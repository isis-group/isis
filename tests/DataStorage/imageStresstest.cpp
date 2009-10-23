#include "DataStorage/image.hpp"
#include <boost/timer.hpp>

using namespace isis;

const size_t slices=100;
const size_t tsteps=100;
const size_t slice_size=256;

int main()
{
	ENABLE_LOG(data::DataLog,util::DefaultMsgPrint,util::info);
	ENABLE_LOG(data::DataDebug,util::DefaultMsgPrint,util::info);
	boost::timer timer;
	data::Image img;

	timer.restart();
	for(size_t tstep=0;tstep< tsteps;tstep++){
		for(size_t slice=0;slice< slices;slice++){
			data::MemChunk<short> chk(slice_size,slice_size);
			chk.setProperty("indexOrigin",util::fvector4(0,0,slice,tstep));
			if(!img.insertChunk(chk))
				std::cout << "Inserting Chunk " << slice << " failed" << std::endl;
		}
	}
	std::cout << tsteps << "*" << slices << " Chunks inserted in " << timer.elapsed() << " sec "<< std::endl;

	timer.restart();
	img.reIndex();
	std::cout << "Image indexed in " << timer.elapsed() << " sec" << std::endl;

	timer.restart();
	for(size_t tstep=0;tstep< tsteps;tstep++)
		for(size_t slice=0;slice< slices;slice++)
			for(size_t phase=0;phase< slice_size;phase++)
				for(size_t read=0;read< slice_size;read++)
					img.voxel<short>(read,phase,slice,tstep)=42;
	
	std::cout << tsteps*slices*slice_size*slice_size << " voxel set to 42 in " << timer.elapsed() << " sec" << std::endl;
	
	return 0;
}