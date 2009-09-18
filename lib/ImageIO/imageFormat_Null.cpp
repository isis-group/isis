#include <DataStorage/io_interface.h>
#include <iostream>

void isis_io_load(const char* filename,const char* dialect){
	std::cout 
		<< "Hello, If I wouldn't be just an empty loader I would now load the file \"" << filename 
		<< "\" using the dialect \"" << dialect << "\"" << std::endl;
}