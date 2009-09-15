
#include <stdio.h>
#include <stdlib.h>

#include "CoreUtils/log.hpp"
#include <iostream>
#include <CoreUtils/message.hpp>


using namespace isis::util; //needed for the log-levels

int main(int argc, char *argv[]){
	ENABLE_LOG(CoreLog,DefaultMsgPrintNeq,info);
	//Enables Logging for FileIO (if FileIO::use_rel is not "0"), makes it use the DefaultMsgPrintNeq-handler and sets the maximal allowed loglevel to 2

	DefaultMsgPrintNeq::setStream(std::cerr);
	DefaultMsgPrintNeq::stopBelow(warning);//will make the process stop at LOG-messages below warning (error)
	//Sets the output stream of the DefaultMsgPrintNeq-handler to std::cerr (instead of std::cout)
	{
		MAKE_LOG(CoreLog);//creates an logging object for FileIO for this codeblock
		LOG(CoreLog,info) << "Hallo " << MSubject("Echo") << std::endl;
		LOG(CoreLog,info) << "Hallo " << MSubject("Otto") <<  std::endl;
		LOG(CoreLog,info) << "Hallo " << MSubject("Otto") <<  std::endl;//wont be displayed, because its the same as before
		LOG(CoreLog,info) << "Hallo " << MSubject("du") << ", wie gehts "  << MSubject("dir") <<  " denn so" <<  std::endl;//multiple subjects
		LOG(CoreLog,verbose_info) << "Hallo " << MSubject("Mutti") <<  std::endl;//wont be processed, because loglevel is only 2
		LOG(CoreLog,error) << "HAAALT!!!11111" <<  std::endl;//will stop the process at this point because 0<1 (stopBelow(1) )
		LOG(CoreLog,info) << "Weiter gehts" <<  std::endl;//will be printed after process was resumed
	}//created logging object will be destroyed here

  return EXIT_SUCCESS;
}
