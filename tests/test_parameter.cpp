

#include "CoreUtils/application.hpp"

using namespace isis;

int main(int argc,char **argv){
	util::Application app("isis program parameter example");

	app.parameters["dump"]=std::string("/tmp/delme");
	app.parameters["dump"].needed()=false;

	app.parameters["offset"]=util::dlist();
	app.init(argc,argv);

	std::cout << "The dump file: "<< app.parameters["dump"].toString() << std::endl;
	std::cout << "The offset as parameter:"<< app.parameters["offset"] << std::endl; //prints as ProgParameter
	std::cout << "The offset as Type:"<< app.parameters["offset"].toString(true) << std::endl; //prints as string from TypeBase::toString
}