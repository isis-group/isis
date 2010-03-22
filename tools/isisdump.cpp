#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include <fstream>

using namespace isis::data;
using namespace isis::util;
using isis::util::DefaultMsgPrint;

int main(int argc, char *argv[])
{
	
/*	ENABLE_LOG(isis::image_io::ImageIoDebug,DefaultMsgPrint,info);
	ENABLE_LOG(isis::image_io::ImageIoLog,DefaultMsgPrint,info);
	ENABLE_LOG(CoreDebug,DefaultMsgPrint,info);
	ENABLE_LOG(CoreLog,DefaultMsgPrint,info);
	ENABLE_LOG(DataDebug,DefaultMsgPrint,warning);
	ENABLE_LOG(DataLog,DefaultMsgPrint,warning);
*/
	
	ImageList images=IOFactory::load(argv[1]);
	unsigned short count1=0,count2=0;
	std::cout << "Got " << images.size() << " Images" << std::endl;
	std::ofstream dump;
	if(argc>2)
		dump.open(argv[2]);
	BOOST_FOREACH(ImageList::const_reference ref,images){
		std::cout << "======Image #" << ++count1 << ref->sizeToString() << "======Metadata======" << std::endl;
		ref->print(std::cout,true);
		for(Image::ChunkIterator c=ref->chunksBegin();c!=ref->chunksEnd();c++){
			std::cout << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << "======Metadata======" << std::endl;
			c->print(std::cout,true);
			if(dump.is_open()){
				dump << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << "======Voxel Data======" << std::endl;
				dump << c->toString() << std::endl;
			}
		}
	}
	return 0;
}
