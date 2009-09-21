#include "io_factory.hpp"
#include <dlfcn.h>
#include <iostream>
#include <CoreUtils/log.hpp>
#include "common.hpp"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace isis{ namespace data{

IOFactory::IOFactory(){
	MAKE_LOG(DataDebug);
	findPlugins(std::string(BUILD_PATH)+ "/lib/ImageIO");
}

bool IOFactory::registerIOClass(boost::shared_ptr< isis::data::FileFormat > plugin){
	MAKE_LOG(DataLog);
	if(!plugin)return false;

	LOG(DataLog,::isis::util::info)<< "Registering io-plugin \"" << plugin->name() << "\" with " << " supported formats" <<  std::endl;

// 	for(std::list<FileFormat::format>::const_iterator i=formats.begin();i!=formats.end();i++)
// 		io_format[i->name]=plugin;

	return true;
}

void IOFactory::findPlugins(std::string path){
	MAKE_LOG(DataLog);
	boost::filesystem::path p(path);
	if (!exists(p)){
		LOG(DataLog,::isis::util::warning) << ::isis::util::MSubject(p.native_file_string()) << " not found" << std::endl;
		return;
	}
	if(!boost::filesystem::is_directory(p)){
		LOG(DataLog,::isis::util::warning) << ::isis::util::MSubject(p.native_file_string()) << " is no directory" << std::endl;
		return;
	}

	boost::regex pluginFilter(std::string("^")+DL_PREFIX+"isisImageFormat_"+"[[:word:]]+"+DL_SUFFIX+"$");

	for (boost::filesystem::directory_iterator itr(p); itr!=boost::filesystem::directory_iterator(); ++itr)	{
		if(boost::regex_match(itr->path().leaf(),pluginFilter)){
			const std::string pluginName=itr->path().string();
			void *handle=dlopen(pluginName.c_str(),RTLD_NOW);
			if(handle){
				isis::data::FileFormat* (*plugin_get_class)() = (isis::data::FileFormat* (*)())dlsym(handle,"factory");
				if (plugin_get_class){
					boost::shared_ptr<isis::data::FileFormat> io_class(plugin_get_class());
					if(!registerIOClass(io_class))
						LOG(DataLog,::isis::util::error) << "invalid image format" << std::endl;
				} else
					LOG(DataLog,::isis::util::error)
						<< "could not get format factory function: " << isis::util::MSubject(dlerror()) << std::endl;
			} else
				LOG(DataLog,::isis::util::error)
					<< "Could not load library: " << isis::util::MSubject(dlerror()) << std::endl;
		} else {
			LOG(DataLog,::isis::util::info)
				<< "Ignoring " << ::isis::util::MSubject(itr->path())
				<< " because it doesn't match " << pluginFilter.str() << std::endl;
		}
	}
}

}}