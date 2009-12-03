
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
		LOG(CoreLog,info) << "Hallo " << MSubject("Echo");
		LOG(CoreLog,info) << "Hallo " << MSubject("Otto");
		LOG(CoreLog,info) << "Hallo " << MSubject("Otto");//wont be displayed, because its the same as before
		LOG(CoreLog,info) << "Hallo " << MSubject("du") << ", wie gehts "  << MSubject("dir") <<  " denn so";//multiple subjects
		LOG(CoreLog,verbose_info) << "Hallo " << MSubject("Mutti");//wont be processed, because loglevel is only 2
		LOG(CoreLog,error) << "HAAALT!!!11111";//will stop the process at this point because 0<1 (stopBelow(1) )
		LOG(CoreLog,info) << "Weiter gehts";//will be printed after process was resumed
	}//created logging object will be destroyed here

  return EXIT_SUCCESS;
}
