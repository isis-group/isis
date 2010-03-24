#include "DataStorage/io_interface.h"
#include "boost/foreach.hpp"
#include "CoreUtils/log.hpp"
#include <boost/filesystem.hpp>
#include "ImageIO/common.hpp"

namespace isis{ namespace image_io{

bool FileFormat::write(const isis::data::ImageList& images, const std::string& filename,const std::string& dialect)
{
	boost::filesystem::path path(filename);
	std::string file=path.leaf();
	path.remove_leaf();
	bool ret=true;
	
	BOOST_FOREACH(data::ImageList::const_reference ref,images){
		if(not ref->hasProperty("sequenceNumber")){
			LOG(Runtime,error)
			<< "sequenceNumber is missing, so I can't generate a unique filename. Won't write...";
			ret=false;
			continue;
		}
		std::string snum=ref->getPropertyValue("sequenceNumber")->toString();
		std::string unique_name=std::string("S")+snum+"_"+file;
		LOG(Runtime,info) 	<< "Writing image to " <<  path/unique_name;
		ret&=write(*ref,(path/unique_name).string(),dialect);
	}
	return ret;
}

bool FileFormat::hasOrTell(const std::string& name, const isis::util::PropMap& object, isis::LogLevel level) {
	if(object.hasProperty(name)){
		return true;
	} else {
		LOG(Runtime,level) << "Missing property " << util::MSubject(name);
		return false;
	}
}

}}
