

#include "CoreUtils/application.hpp"

using namespace isis;

int main(int argc,char **argv){
	util::Application app("isis program parameter example");
	app.init(argc,argv);
}