#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include <fstream>

using namespace isis;

int main(int argc, char *argv[])
{
	
	ENABLE_LOG(ImageIoLog,util::DefaultMsgPrint,error);
	ENABLE_LOG(DataLog,util::DefaultMsgPrint,error);

	
	data::ImageList images=data::IOFactory::load(argv[1]);
	unsigned short count1=0,count2=0;
	std::cout << "Got " << images.size() << " Images" << std::endl;
	std::ofstream dump;
	if(argc>2)
		dump.open(argv[2]);
	BOOST_FOREACH(data::ImageList::const_reference ref,images){
		std::cout << "======Image #" << ++count1 << ref->sizeToString() << "======Metadata======" << std::endl;
		ref->print(std::cout,true);
		for(data::Image::ChunkIterator c=ref->chunksBegin();c!=ref->chunksEnd();c++){
			std::cout << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << c->typeName() << "======Metadata======" << std::endl;
			c->print(std::cout,true);
			if(dump.is_open()){
				dump << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << c->typeName() << "======Voxel Data======" << std::endl;
				dump << c->toString() << std::endl;
			}
		}
	}
	return 0;
}
