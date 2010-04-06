

#include "CoreUtils/progparameter.hpp"

using namespace isis;

int main(int argc,char **argv){
	util::ParameterMap params;
	params["help"]=false;
	params["help"].setDescription("Displays help");

	params["in"]=std::string();
	params["in"].setDescription("Input file");
	
	params["dCore"]=util::Selection("error,warning,info,verbose_info");
	params["dCore"].setDescription("Set debugging level for Core");
	
	params["dump"]=std::string("/tmp/dump.txt");
	params["dump"].needed()=false;

	params.parse(argc,argv);

	if(params["help"])
	{
		std::cout << "Help for program parameter example. Valid parameters are:"<< std::endl;
		params.printAll();
		exit(0);
	}
	if(not params.isComplete()){
		std::cout << "Missing parameters:"<< std::endl;
		params.printNeeded();
		std::cout << "Exiting" << std::endl;
		exit(1);
	}
}