#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include <fstream>
#include "CoreUtils/progparameter.hpp"

using namespace isis;

int main(int argc, char *argv[])
{
	util::enable_log<util::DefaultMsgPrint>(verbose_info);

	util::ParameterMap params;
	params["in"]=std::string();
	params["dump"]=std::string();
	params["dump"].needed()=false;
	params.parse(argc,argv);
	if(not params.isComplete()){
		std::cout << "Missing parameters:" << std::endl;
		params.printNeeded();
		std::cout << "Valid parameters are " << std::endl;
		params.printAll();
		exit(0);
	}
	
	data::ImageList images=data::IOFactory::load(params["in"]->as<std::string>());
	unsigned short count1=0,count2=0;
	std::cout << "Got " << images.size() << " Images" << std::endl;
	std::ofstream dump;
	if(argc>2)
		dump.open(params["dump"]->as<std::string>().c_str());
	BOOST_FOREACH(data::ImageList::const_reference ref,images){
		std::cout << "======Image #" << ++count1 << ref->sizeToString() << "======Metadata======" << std::endl;
		ref->print(std::cout,true);
		for(data::Image::ChunkIterator c=ref->chunksBegin();c!=ref->chunksEnd();c++){
			std::cout << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << c->typeName() << "======Metadata======" << std::endl;
			c->print(std::cout,true);
			if(dump.is_open()){
				dump << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << c->typeName() << "======Voxel Data======" << std::endl;
				dump << c->getTypePtrBase().toString() << std::endl;
			}
		}
	}
	return 0;
}
