

#include "CoreUtils/application.hpp"

using namespace isis;

int main(int argc,char **argv){
	util::Application app("isis program parameter example");

	app.parameters["dump"]=std::string("/tmp/delme");
	app.parameters["dump"].needed()=false;
	app.init(argc,argv);

	std::cout << app.parameters["dump"].toString() << std::endl;
}