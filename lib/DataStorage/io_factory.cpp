#include "io_factory.hpp"
#include <dlfcn.h>
#include <iostream>
#include <CoreUtils/log.hpp>
#include "common.hpp"

namespace isis{ namespace data{

IOFactory::IOFactory(std::string path){
	MAKE_LOG(DataDebug);
	std::string libname="isisImageFormat_Null";
	libname = path + "/" + DL_PREFIX + libname + DL_SUFFIX;
	
	void *handle=dlopen(libname.c_str(),RTLD_NOW);
	if(handle){
		void (*plugin_load)(const char* filename,const char* dialect) = 
			(void (*)(const char* filename,const char* dialect))dlsym(handle,"isis_io_load");
		if (plugin_load)
			io_func["null"]=plugin_load;
		else
			LOG(DataDebug,::isis::util::error) 
				<< "could not bind function: " << isis::util::MSubject(dlerror()) << std::endl;

	} else
		LOG(DataDebug,::isis::util::error) 
			<< "Could not load lib: " << isis::util::MSubject(dlerror()) << std::endl;
}

void IOFactory::do_load(std::string filename,std::string dialect){
	MAKE_LOG(DataDebug);
	std::string type="null";
	//@todo insert nifty file-type-detection here
	if(io_func[type])
		io_func[type](filename.c_str(),dialect.c_str());
	else
		LOG(DataDebug,::isis::util::error)
			<< "There is no loader available for " << isis::util::MSubject(filename) << std::endl;
}

}}