#ifndef IO_FACTORY_H
#define IO_FACTORY_H

#include <map>
#include <string>
#include "io_interface.h"
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

namespace isis{ namespace data{

class IOFactory{
	std::map<std::string,boost::shared_ptr< isis::data::FileFormat> > io_format;
	std::map<std::string,std::string> io_suffix;
	bool registerIOClass(boost::shared_ptr< isis::data::FileFormat > plugin);
	void findPlugins(std::string path);
public:
	IOFactory();
};

	
}}

#endif //IO_FACTORY_H