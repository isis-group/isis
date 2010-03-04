#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>

using namespace isis::data;
using namespace isis::util;
using isis::util::DefaultMsgPrint;

int main(int argc, char *argv[])
{
	int ret=0;
	if(argc<3){
		std::cout << "Call " << argv[0] << " <first dataset> <second dataset>"<< std::endl;
		return -1;
	}
	ImageList images1=IOFactory::load(argv[1]);
	ImageList images2=IOFactory::load(argv[2]);

	if(images1.size() != images2.size()){
		std::cout << "Amount of found images in " << argv[1] << " and " << argv[2] << " differs" << std::endl;
		return -1;
	}

	std::cout << "Got " << images1.size() << " Images from " << argv[1] << " and " << argv[2] << std::endl;

	ImageList::const_iterator i,j;
	int count;
	for(i=images1.begin(),j=images2.begin(),count=0;i!=images1.end();i++,j++,count++){
		const Image &first=**i,&second=**j;
		const PropMap::diff_map diff=first.diff(second);
		ret+=diff.size();
		if(not diff.empty()){
			std::cout
			<< "Metadata of " << argv[1] << ":" << count << " and "
			<< argv[2] << ":" << count  << " differ:" << std::endl
			<< diff	<< std::endl;
		}
		if(first.sizeToVector() == second.sizeToVector()){
			std::cout 
			<< "Image sizes of " << argv[1] << ":" << count << " and "
			<< argv[2] << ":" << count  << " differ:" 
			<< first.sizeToString()	<< "/" << second.sizeToString() << std::endl;
			ret++;
		}
	}
	/*	BOOST_FOREACH(ImageList::const_reference ref,images){
		std::cout << "======Image #" << ++count1 << ref->sizeToString() << "======Metadata======" << std::endl;
		ref->print(std::cout,true);
		for(Image::ChunkIterator c=ref->chunksBegin();c!=ref->chunksEnd();c++){
			std::cout << "======Image #" <<count1 << "==Chunk #" << ++count2 << c->sizeToString() << "======Metadata======" << std::endl;
			c->print(std::cout,true);
		}
	}*/
	return ret;
}
