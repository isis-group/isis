
#include <stdio.h>
#include <stdlib.h>

#include "CoreUtils/log.hpp"
#include <iostream>


using iUtil::CoreLog;//needed because using namespaces in the logging macros will break them

int main(int argc, char *argv[]){
	ENABLE_LOG(CoreLog,iUtil::DefaultMsgPrintNeq,2);
	//Enables Logging for FileIO (if FileIO::use_rel is not "0"), makes it use the DefaultMsgPrintNeq-handler and sets the maximal allowed loglevel to 2

	iUtil::DefaultMsgPrintNeq::setStream(std::cerr);
	iUtil::DefaultMsgPrintNeq::stopBelow(1);//will make the process stop at LOG-messages below 1 
	//Sets the output stream of the DefaultMsgPrintNeq-handler to std::cerr (instead of std::cout)
	{
		MAKE_LOG(CoreLog);//creates an logging object for FileIO for this codeblock
		LOG(CoreLog,2) << "Hallo " << iUtil::MSubject("Echo") << std::endl;
		LOG(CoreLog,2) << "Hallo " << iUtil::MSubject("Otto") <<  std::endl;
		LOG(CoreLog,2) << "Hallo " << iUtil::MSubject("Otto") <<  std::endl;//wont be displayed, because its the same as before
		LOG(CoreLog,2) << "Hallo " << iUtil::MSubject("du") << ", wie gehts "  << iUtil::MSubject("dir") <<  " denn so" <<  std::endl;//multiple subjects
		LOG(CoreLog,3) << "Hallo " << iUtil::MSubject("Mutti") <<  std::endl;//wont be processed, because loglevel is only 2
		LOG(CoreLog,0) << "HAAALT!!!11111" <<  std::endl;//will stop the process at this point because 0<1 (stopBelow(1) )
		LOG(CoreLog,2) << "Weiter gehts" <<  std::endl;//will be printed after process was resumed
	}//created logging object will be destroyed here

  return EXIT_SUCCESS;
}
