#ifndef IO_FACTORY_H
#define IO_FACTORY_H

#include <map>
#include <string>

namespace isis{ namespace data{

class IOFactory{
	std::map<std::string,void (*)(const char* filename,const char* dialect)> io_func;
public:
	IOFactory(std::string path);
	void do_load(std::string filename,std::string dialect);
};

	
}}

#endif //IO_FACTORY_H