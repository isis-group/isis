//
// C++ Interface: io_factory
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//



#ifndef IO_FACTORY_H
#define IO_FACTORY_H

#include <map>
#include <string>
#include "io_interface.h"
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>

//????
#include "chunk.hpp"
#include "image.hpp"

namespace isis {
namespace data {

class IOFactory
{
public:
	typedef boost::shared_ptr< ::isis::image_io::FileFormat> FileFormatPtr;
	typedef std::list<FileFormatPtr> FileFormatList;
private:
	typedef std::map<std::string, FileFormatPtr> FormatFormatMap;
public:
	/**
	 * load a data file with given filename and dialect
	 * @param ret ChunkList to store the loaded chunks in
	 * @param filename file to open
	 * @param dialect dialect of the fileformat to load
	 * @return list of chunks (part of an image)
	 */
	int loadFile(ChunkList& ret,const boost::filesystem::path& filename, const std::string& dialect);
	int loadPath(ChunkList& ret,const boost::filesystem::path& path, const std::string& dialect);
	/**
	 * get all file suffixes a plugin suggests to handle
	 * @param reader the plugin to ask
	 * @return a list of suffixes the plugin handles
	 */
	static std::list<std::string> getSuffixes(const FileFormatPtr& reader);

	static ImageList load(const std::string& path, std::string dialect="");
	static bool write(const ImageList &images,const std::string& path, const std::string& dialect);
	
	template<typename charT, typename traits> static void print_formats(std::basic_ostream<charT, traits> &out)
	{
		for(std::list<FileFormatPtr>::const_iterator it = get().io_formats.begin(); it != get().io_formats.end(); it++)
			out << (*it)->name() << std::endl;
	}

	static IOFactory &get();
protected:
	IOFactory();//shall not be created directly
	FileFormatList io_formats;

	/*
	 * each ImageFormat will be registered in a map after plugin has been loaded
	 * @param plugin pointer to the plugin to register
	 *
	 * @return true if registration was successful, false otherwise
	 * */
	bool registerFormat(const FileFormatPtr plugin);
	unsigned int findPlugins(const std::string& path);
	FileFormatList getFormatInterface(const std::string& filename, const std::string& dialect);
private:
	std::map<std::string, FileFormatList,util::_internal::caselessStringLess> io_suffix;
	IOFactory& operator =(IOFactory&); //dont do that
};

}
}

#endif //IO_FACTORY_H
