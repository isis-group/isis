#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include "CoreUtils/application.hpp"

using namespace isis;

int main(int argc, char *argv[])
{
	util::Application app("isis data diff");
	
	app.parameters["ignore"]=util::slist();
	app.parameters["ignore"].needed()=false;
	app.parameters["ignore"].setDescription("List of properties which should be ignored when comparing");
	app.parameters["in"]=util::slist();
	app.parameters["in"].setDescription("The two files (or directories) which you want to compare");
	if(not app.init(argc,argv))
		return 1;
	
	int ret=0;
	util::slist files=app.parameters["in"];
	if(files.size()!=2){
		std::cout << "You'll have to give two input files" << std::endl << "(but you gave "<< files << ")" << std::endl;
		return 1;
	}
	
	data::ImageList images1=data::IOFactory::load(files.front());
	data::ImageList images2=data::IOFactory::load(files.back());

	if(images1.size() != images2.size()){
		std::cout << "Amount of found images in " << files.front() << " and " << files.back() << " differs" << std::endl;
		return -1;
	}

	data::ImageList::const_iterator i,j;
	int count;
	util::slist ignore=app.parameters["ignore"];
	ignore.push_back("source");
	
	for(i=images1.begin(),j=images2.begin(),count=0;i!=images1.end();i++,j++,count++){
		const data::Image &first=**i,&second=**j;
		util::PropMap::diff_map diff=first.diff(second);
		BOOST_FOREACH(util::slist::const_reference ref,ignore)
			diff.erase(ref);		
		
		ret+=diff.size();
		if(not diff.empty()){
			std::cout
			<< "Metadata of " << files.front() << ":" << count << " and "
			<< files.back() << ":" << count  << " differ:" << std::endl
			<< diff	<< std::endl;
		}
		if(first.sizeToVector() != second.sizeToVector()){
			std::cout 
			<< "Image sizes of " << files.front() << ":" << count << " and "
			<< files.back() << ":" << count  << " differ:"
			<< first.sizeToString()	<< "/" << second.sizeToString() << std::endl;
			ret++;
		} else {
			size_t voxels=first.cmp(second);
			if(voxels != 0){
				std::cout << voxels
				<< " of " << first.sizeToVector().product() << " voxels in " << files.front() << ":" << count << " and "
				<< files.back() << ":" << count  << " differ" << std::endl;
				ret++;
			}
		}
	}
	return ret;
}
