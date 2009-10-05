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

//????
#include "chunk.hpp"

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
	/*get all file suffixes a plugin suggests to handle
	 * @param reader the plugin to ask
	 *
	 * @return a list of suffixes the plugin handles
	 * */
	static std::list<std::string> getSuffixes(
	    const FileFormatPtr& reader);

	/*
	 * load a data file with given filename and dialect
	 *@params filename file to open
	 *@params dialect dialect of the fileformat to load
	 *@return list of chunks (part of an image)
	 * */
	ChunkList loadFile(
		const std::string& filename, const std::string& dialect);


	template<typename charT, typename traits> void print_formats(
	    std::basic_ostream<charT, traits> &out) {
		for(std::list<FileFormatPtr>::const_iterator it = io_formats.begin(); it != io_formats.end(); it++) {
			out << (*it)->name() << std::endl;
		}
	}

	static IOFactory &get();
protected:
	FileFormatList io_formats;

	/*
	 * each ImageFormat will be registered in a map after plugin has been loaded
	 * @param plugin pointer to the plugin to register
	 *
	 * @return true if registration was successful, false otherwise
	 * */
	bool registerFormat(FileFormatPtr plugin);
	unsigned int findPlugins(std::string path);
	FileFormatList getFormatReader(const std::string& filename, const std::string& dialect);
private:
	std::map<std::string, FileFormatList> io_suffix;
	IOFactory();//shall not be created directly
	IOFactory& operator =(IOFactory&); //dont do that
};

}
}

#endif //IO_FACTORY_H
