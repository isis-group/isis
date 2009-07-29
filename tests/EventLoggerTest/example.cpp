
#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include <iostream>

struct FileIO{
  enum {use_rel = _ENABLE_FILEIO_LOG};
};

int main(int argc, char *argv[]){
	ENABLE_LOG(FileIO,DefaultMsgPrintNeq,2);
	//Enables Logging for FileIO (if FileIO::use_rel is not "0"), makes it use the DefaultMsgPrintNeq-handler and sets the maximal allowed loglevel to 2

	DefaultMsgPrintNeq::setStream(std::cerr);
	//Sets the output stream of the DefaultMsgPrintNeq-handler to std::cerr (instead of std::cout)
	{
		MAKE_LOG(FileIO);//creates an logging object for FileIO for this codeblock
		LOG(FileIO,2) << "Hallo " << MSubject("Echo") <<  std::endl;
		LOG(FileIO,2) << "Hallo " << MSubject("Otto") <<  std::endl;
		LOG(FileIO,2) << "Hallo " << MSubject("Otto") <<  std::endl;//wont be displayed, because its the same as before
		LOG(FileIO,2) << "Hallo " << MSubject("du") << ", wie gehts denn so" <<  std::endl;
		LOG(FileIO,3) << "Hallo " << MSubject("Mutti") <<  std::endl;//wont be processed, because loglevel is only 2
	}//created logging object will be destroyed here

  return EXIT_SUCCESS;
}
