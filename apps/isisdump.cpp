#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>

using namespace isis::data;
using namespace isis::util;
using isis::util::DefaultMsgPrint;

int main(int argc, char *argv[])
{
	
	ENABLE_LOG(isis::image_io::ImageIoDebug,DefaultMsgPrint,warning);
	ENABLE_LOG(isis::image_io::ImageIoLog,DefaultMsgPrint,warning);
	ENABLE_LOG(CoreDebug,DefaultMsgPrint,info);
	ENABLE_LOG(CoreLog,DefaultMsgPrint,info);
	ENABLE_LOG(DataDebug,DefaultMsgPrint,warning);
	ENABLE_LOG(DataLog,DefaultMsgPrint,warning);
	
	ImageList images=IOFactory::load(argv[1]);
	unsigned short count=0;
	std::cout << "Got " << images.size() << " Images" << std::endl;
	BOOST_FOREACH(ImageList::const_reference ref,images){
		std::cout << "======Image " << ++count << "======Metadata======" << std::endl;
		ref->print(std::cout,true) << std::endl;
	}
	return 0;
}
