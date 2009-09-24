
#include <stdio.h>
#include <stdlib.h>
#include "CoreUtils/property.hpp"
#include "CoreUtils/propmap.hpp"
#include <iostream>
#include "CoreUtils/log.hpp"
#include "CoreUtils/vector.hpp"

using namespace isis::util; //needed for the log-levels

int main(int argc, char *argv[]){

	ENABLE_LOG(CoreLog,DefaultMsgPrint,info);

	PropertyValue a=std::string("Hallo");
	std::cout << a->toString(true) << std::endl;

	PropertyValue b=a;
	std::cout << b->toString(true) << std::endl;

	b=5.2;
	std::cout << b->toString(true) << std::endl;

	PropMap map,cont;
	map["Test1"]=6.4;
	map["Test2"]=6;
	map["Test3"]=std::string("Hallo");
// 	map["Vector"]=FixedVector<short,4>();
	
	cont["Prop"]=map;
	std::cout << cont["Prop"]->toString() << std::endl;

	std::string x=map["Test3"];
	std::cout << x << std::endl;

	//will get you int() ("0")
	int fail=map["Test3"];
	
	//will raise bad_cast (Because "Hallo" cannot lexically be casted to "3"
// 	fail=map["Test3"]->as<int>();

	//will be ok ("3" can be lexically casted to int)
	map["Test3"]=std::string("3");
	int ok=map["Test3"]->as<int>();
	
	return EXIT_SUCCESS;
}
