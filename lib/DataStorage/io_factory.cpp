#include "io_factory.hpp"
#include <dlfcn.h>
#include <iostream>
#include "CoreUtils/log.hpp"
#include "common.hpp"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/foreach.hpp>

namespace isis{ namespace data{

IOFactory::IOFactory(){
	MAKE_LOG(DataDebug);
	findPlugins(std::string(BUILD_PATH));
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
		if(boost::filesystem::is_directory(*itr))continue;
		if(boost::regex_match(itr->path().leaf(),pluginFilter)){
			const std::string pluginName=itr->path().string();
			void *handle=dlopen(pluginName.c_str(),RTLD_NOW);
			if(handle){
				isis::image_io::FileFormat* (*factory_func)() = (isis::image_io::FileFormat* (*)())dlsym(handle,"factory");
				if (factory_func){
					FileFormatPtr io_class(factory_func());
					if(registerFormat(io_class))
						ret++;
					else
						LOG(DataLog,::isis::util::error) << "failed to register plugin " << isis::util::MSubject(pluginName) << std::endl;
					  
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

std::list<std::string> IOFactory::getSuffixes(const FileFormatPtr& reader){
	const boost::sregex_token_iterator token_begin=boost::make_regex_token_iterator(reader->suffixes(), boost::regex("\\s+"), -1);
	const boost::sregex_token_iterator token_end;

	return std::list<std::string>(token_begin,token_end);
}

IOFactory& IOFactory::get(){
	static IOFactory ret;
	return ret;
}

isis::data::ChunkList IOFactory::loadFile(
		const std::string& filename, const std::string& dialect){
	MAKE_LOG(DataLog);

	FileFormatList formatReader = getFormatReader(filename, dialect);
	if(true == formatReader.empty()){//no suitable plugin
		LOG(DataLog,::isis::util::error)
				<< "Missing plugin to open file: " << filename << " with dialect: " << dialect << std::endl;
		return isis::data::ChunkList();
	}

	for(FileFormatList::const_iterator it = formatReader.begin(); it != formatReader.end(); it++) {
		isis::data::ChunkList loadedChunks = (*it)->load(filename, dialect);
		if (false == loadedChunks.empty()){//load succesfully
			LOG(DataLog,::isis::util::info)
					<< "plugin to load file " <<  filename << " : " << (*it)->name() << " with dialect: " << dialect << std::endl;
			return loadedChunks;
		}
	}
	LOG(DataLog,::isis::util::error)
		<< "Could not open file: " << filename <<std::endl;
	return isis::data::ChunkList();//no plugin of proposed list could load file
}

IOFactory::FileFormatList IOFactory::getFormatReader(const std::string& filename, const std::string& dialect){

	MAKE_LOG(DataLog);
	size_t pos = filename.find_first_of(".", 1);
	if (std::string::npos == pos){
		return FileFormatList();}
	std::string ext = filename.substr(pos);

	if(true == dialect.empty()){//give back whole list of plugins for this file extension
		return io_suffix[ext];
	}
	//otherwise sort out by dialect
	FileFormatList reader;
	for ( FileFormatList::const_iterator it = io_suffix[ext].begin(); it != io_suffix[ext].end(); it++){
		if(std::string::npos != (*it)->dialects().find(dialect)){
			reader.push_back(*it);}
	}
	return reader;
}

}} // namespaces data isis
