#include "io_factory.hpp"
#include <dlfcn.h>
#include <iostream>
#include <CoreUtils/log.hpp>
#include "common.hpp"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>

namespace isis{ namespace data{

IOFactory::IOFactory(){
	MAKE_LOG(DataDebug);
	findPlugins(std::string(BUILD_PATH)+ "/lib/ImageIO");
}

bool IOFactory::registerFormat(FileFormatPtr plugin){
	MAKE_LOG(DataLog);
	if(!plugin)return false;

	io_formats.push_back(plugin);
	std::list<std::string> suffixes=getSuffixes(plugin);
	LOG(DataLog,::isis::util::info)
		<< "Registering " << (plugin->tainted() ? "tainted " :"") << "io-plugin \"" 
		<< ::isis::util::MSubject(plugin->name())
		<< "\" with " << suffixes.size() << " supported suffixes" << std::endl;

	BOOST_FOREACH(std::string &it,suffixes)
		io_suffix[it].push_back(plugin);

	return true;
}

unsigned int IOFactory::findPlugins(std::string path){
	MAKE_LOG(DataLog);
	boost::filesystem::path p(path);
	if (!exists(p)){
		LOG(DataLog,::isis::util::warning) << ::isis::util::MSubject(p.native_file_string()) << " not found" << std::endl;
		return 0;
	}
	if(!boost::filesystem::is_directory(p)){
		LOG(DataLog,::isis::util::warning) << ::isis::util::MSubject(p.native_file_string()) << " is no directory" << std::endl;
		return 0;
	}

	boost::regex pluginFilter(std::string("^")+DL_PREFIX+"isisImageFormat_"+"[[:word:]]+"+DL_SUFFIX+"$");
	unsigned int ret=0;
	for (boost::filesystem::directory_iterator itr(p); itr!=boost::filesystem::directory_iterator(); ++itr)	{
		if(boost::regex_match(itr->path().leaf(),pluginFilter)){
			const std::string pluginName=itr->path().string();
			void *handle=dlopen(pluginName.c_str(),RTLD_NOW);
			if(handle){
				isis::data::FileFormat* (*factory_func)() = (isis::data::FileFormat* (*)())dlsym(handle,"factory");
				if (factory_func){
					FileFormatPtr io_class(factory_func());
					if(registerFormat(io_class))
						ret++;
					else
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
	return ret;
}

std::list<std::string> IOFactory::getSuffixes(FileFormatPtr reader){
	const boost::sregex_token_iterator token_begin=boost::make_regex_token_iterator(reader->suffixes(), boost::regex("\\s+"), -1);
	const boost::sregex_token_iterator token_end;

	return std::list<std::string>(token_begin,token_end);
}

IOFactory& IOFactory::get(){
	static IOFactory ret;
	return ret;
}

}}