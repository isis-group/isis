#ifndef IO_FACTORY_H
#define IO_FACTORY_H

#include <map>
#include <string>
#include "io_interface.h"
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

namespace isis{ namespace data{

class IOFactory{
	IOFactory();//shall not be created directly
	IOFactory& operator =(IOFactory&); //dont do that
public:
	typedef boost::shared_ptr< FileReader> FileReaderPtr;
	typedef std::list< FileReaderPtr > FileReaderList;
	
	static std::list<std::string> getSuffixes(FileReaderPtr reader);
	static IOFactory &get();
protected:
	FileReaderList io_reader;
	std::map<std::string,FileReaderList > io_suffix;

	bool registerReader(FileReaderPtr plugin);
	unsigned int findPlugins(std::string path);
};

}}

#endif //IO_FACTORY_H