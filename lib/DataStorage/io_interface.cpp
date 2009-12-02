#include <DataStorage/io_interface.h>
#include <boost/foreach.hpp>
#include <CoreUtils/log.hpp>
#include "boost/filesystem.hpp"

using isis::data::DataLog;

bool isis::image_io::FileFormat::write(const isis::data::ImageList& images, std::string filename, std::string dialect)
{
	MAKE_LOG(DataLog);
	boost::filesystem::path path(filename);
	filename=path.leaf();
	path.remove_leaf();
	bool ret=true;
	
	BOOST_FOREACH(data::ImageList::const_reference ref,images){
		if(not ref->hasProperty("sequenceNumber")){
			LOG(DataLog,util::error)
			<< "sequenceNumber is missing, so I can't generate a unique filename. Won't write..." << std::endl;
			ret=false;
			continue;
		}
		std::string snum=ref->getPropertyValue("sequenceNumber")->toString();
		std::string unique_name=std::string("S")+snum+"_"+filename;
		LOG(DataLog,util::info) 	<< "Writing image to " <<  path/unique_name << std::endl;
		ret&=write(*ref,(path/unique_name).string(),dialect);
	}
	return ret;
}
