#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>

using namespace isis::data;
using isis::util::DefaultMsgPrint;

int main(int argc, char *argv[])
{
	
	ENABLE_LOG(isis::image_io::ImageIoLog,DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(isis::image_io::ImageIoLog,DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(DataLog,DefaultMsgPrint,isis::util::warning);
	ENABLE_LOG(DataDebug,DefaultMsgPrint,isis::util::warning);
	
	ImageList images=IOFactory::load(argv[1]);
	unsigned short count=0;
	std::cout << "Got " << images.size() << " Images" << std::endl;
	BOOST_FOREACH(ImageList::const_reference ref,images){
		std::cout << "======Image " << ++count << "======Metadata======" << std::endl;
		ref->print(std::cout,true) << std::endl;
	}
	return 0;
}
