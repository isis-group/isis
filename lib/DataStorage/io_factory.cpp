//
// C++ Implementation: io_factory
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "io_factory.hpp"
#include <dlfcn.h>
#include <iostream>
#include "CoreUtils/log.hpp"
#include "common.hpp"
#include <boost/regex.hpp>
#include <boost/foreach.hpp>

namespace isis{ namespace data{

IOFactory::IOFactory()
{
	MAKE_LOG(DataDebug);
	findPlugins(std::string(BUILD_PATH));
}

bool IOFactory::registerFormat(FileFormatPtr plugin)
{
	MAKE_LOG(DataLog);
	if(!plugin)return false;

	io_formats.push_back(plugin);
	std::list<std::string> suffixes=getSuffixes(plugin);
	LOG(DataLog,util::info)
		<< "Registering " << (plugin->tainted() ? "tainted " :"") << "io-plugin \"" 
		<< util::MSubject(plugin->name())
		<< "\" with " << suffixes.size() << " supported suffixes" << std::endl;

	BOOST_FOREACH(std::string &it,suffixes)
		io_suffix[it].push_back(plugin);

	return true;
}

unsigned int IOFactory::findPlugins(std::string path)
{
	MAKE_LOG(DataLog);
	boost::filesystem::path p(path);
	if (!exists(p)){
		LOG(DataLog,util::warning) << util::MSubject(p.native_file_string()) << " not found" << std::endl;
		return 0;
	}
	if(!boost::filesystem::is_directory(p)){
		LOG(DataLog,util::warning) << util::MSubject(p.native_file_string()) << " is no directory" << std::endl;
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
				image_io::FileFormat* (*factory_func)() = (image_io::FileFormat* (*)())dlsym(handle,"factory");
				if (factory_func){
					FileFormatPtr io_class(factory_func());
					if(registerFormat(io_class))
						ret++;
					else
						LOG(DataLog,util::error) << "failed to register plugin " << util::MSubject(pluginName) << std::endl;
					  
				} else
					LOG(DataLog,util::error)
						<< "could not get format factory function: " << util::MSubject(dlerror()) << std::endl;
			} else
				LOG(DataLog,util::error)
					<< "Could not load library: " << util::MSubject(dlerror()) << std::endl;
		} else {
			LOG(DataLog,util::info)
				<< "Ignoring " << util::MSubject(itr->path())
				<< " because it doesn't match " << pluginFilter.str() << std::endl;
		}
	}
	return ret;
}

std::list<std::string> IOFactory::getSuffixes(const FileFormatPtr& reader)
{
	const boost::sregex_token_iterator token_begin=boost::make_regex_token_iterator(reader->suffixes(), boost::regex("\\s+"), -1);
	const boost::sregex_token_iterator token_end;

	return std::list<std::string>(token_begin,token_end);
}

IOFactory& IOFactory::get()
{
	static IOFactory ret;
	return ret;
}

data::ChunkList IOFactory::loadFile(const boost::filesystem::path& filename, const std::string& dialect)
{
	MAKE_LOG(DataLog);
	MAKE_LOG(DataDebug);
	
	FileFormatList formatReader = getFormatInterface(filename.string(), dialect);
	
	if(true == formatReader.empty()){//no suitable plugin
		LOG(DataLog,util::error)
				<< "Missing plugin to open file: " << filename << " with dialect: " << dialect << std::endl;
		return data::ChunkList();
	}

	for(FileFormatList::const_iterator it = formatReader.begin(); it != formatReader.end(); it++) {
		LOG(DataDebug,util::info) << "plugin to load file " <<  filename << ": " << (*it)->name();
		if(dialect=="")LOG(DataDebug,util::info) << " with dialect: " << dialect;
		LOG(DataDebug,util::info) << std::endl; //@fixme log is not printed
		
		const data::ChunkList loadedChunks = (*it)->load(filename.string(), dialect);
		if (not loadedChunks.empty()){//load succesfully
			LOG(DataLog,util::verbose_info)
				<< "loaded " << loadedChunks.size() << " Chunks from " <<  filename << std::endl;
			return loadedChunks;
		}
	}
	LOG(DataLog,util::error)
		<< "Could not open file: " << filename << std::endl; //@todo error message missing
	return data::ChunkList();//no plugin of proposed list could load file
}

IOFactory::FileFormatList IOFactory::getFormatInterface(const std::string& filename, const std::string& dialect)
{
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


data::ImageList IOFactory::load(const std::string& path, const std::string& dialect)
{
	MAKE_LOG(DataLog);
	const boost::filesystem::path p(path);
	const ChunkList chunks =boost::filesystem::is_directory(p) ?
		get().loadPath(p,dialect):
		get().loadFile(p,dialect);
	const data::ImageList images(chunks);
	LOG(DataLog,util::info)
		<< "Loaded " << images.size() << " images from " << p << std::endl;
	return images;
}

ChunkList IOFactory::loadPath(const boost::filesystem::path& path, const std::string& dialect)
{
	MAKE_LOG(DataDebug);
	ChunkList ret;
	for (boost::filesystem::directory_iterator itr(path); itr!=boost::filesystem::directory_iterator(); ++itr)	{
		if(boost::filesystem::is_directory(*itr))continue;
		const ChunkList buff=loadFile(itr->path(),dialect);
		ret.insert(ret.end(),buff.begin(),buff.end());
	}
	LOG(DataDebug,util::info)
		<< "Got " << ret.size() << " Chunks from loading directory " << path << std::endl;
	return ret;
}

bool IOFactory::write(const isis::data::ImageList& images, const std::string& filename, const std::string& dialect)
{
	MAKE_LOG(DataLog);
	MAKE_LOG(DataDebug);
	
	FileFormatList formatWriter = get().getFormatInterface(filename, dialect);
	
	if(formatWriter.empty()){//no suitable plugin
		LOG(DataLog,util::error)
		<< "Missing plugin to write file: " << filename << " with dialect: " << dialect << std::endl;
		return false;
	}
	
	BOOST_FOREACH(FileFormatList::const_reference it,formatWriter) {
		LOG(DataDebug,util::info)
			<< "plugin to write file " <<  filename << ": "
			<< it->name() << std::endl; 
		
		if (it->write(images,filename, dialect)){//succesfully written
			LOG(DataDebug,util::info) << images.size() << " images written using " <<  it->name() << std::endl; 
			return true;
		} else
			LOG(DataLog,util::error)
			<< " could not write " <<  images.size()
			<< " images using " <<  it->name() << std::endl;
	}
	LOG(DataLog,util::error)
		<< "Could not write to: " << filename << std::endl; //@todo error message missing
	return false;
}


}} // namespaces data isis
