#ifndef IO_FACTORY_H
#define IO_FACTORY_H

#include <map>
#include <string>
#include "io_interface.h"
#include <boost/shared_ptr.hpp>
#include <boost/regex.hpp>

namespace isis{ namespace data{

class IOFactory{
public:
	typedef boost::shared_ptr< FileFormat> FileFormatPtr;
	typedef std::list< FileFormatPtr > FileFormatList;
	
	static std::list<std::string> getSuffixes(FileFormatPtr reader);

	template<typename charT, typename traits> void
	print_formats(std::basic_ostream<charT,traits> &out){
		for(std::list< FileFormatPtr >::const_iterator it=io_formats.begin();it!=io_formats.end();it++){
			out << (*it)->name() << std::endl;
		}
	}

	static IOFactory &get();
protected:
	FileFormatList io_formats;

	bool registerFormat(FileFormatPtr plugin);
	unsigned int findPlugins(std::string path);
private:
	std::map<std::string,FileFormatList > io_suffix;
	IOFactory();//shall not be created directly
	IOFactory& operator =(IOFactory&); //dont do that
};

}}

#endif //IO_FACTORY_H