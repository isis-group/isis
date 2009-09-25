/**  The factory that loads and generates image formats from all available plugins
 *
 * #include "ImageFormatFactory.h" <BR>
 * -ldl
 *
 * @see
 *
 */
/*
 * io_factory.hpp
 *
 *  Created on: Sep 18, 2009
 *      Author: reimer
 */


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
	typedef boost::shared_ptr<FileFormat> FileFormatPtr;
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

	/*??NUR EINS VON BEIDEN FOLGENDEN BENOETIGT
	 * create ImageFormats
	 *@params strFormatId the unique id of an image format
	 *
	 *@return pointer to an object of an image format
	 * */
	FileFormatPtr createImageFormat(
		const std::string& strFormatId);

	/*
	 * create Image
	 *@params strFilename file to open
	 *
	 *@return image object
	 * */
	Chunks createImage(
		const std::string& strFilename);


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
	bool registerFormat(
	    FileFormatPtr plugin);
	unsigned int findPlugins(
	    std::string path);
private:
	std::map<std::string, FileFormatList> io_suffix;
	IOFactory();//shall not be created directly
	IOFactory& operator =(
	    IOFactory&); //dont do that
};

}
}

#endif //IO_FACTORY_H
