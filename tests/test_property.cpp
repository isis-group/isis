
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "property.hpp"
#include "propmap.hpp"
#include <iostream>
#include "log.hpp"

int main(int argc, char *argv[]){

  ENABLE_LOG(isis::CoreLog,isis::DefaultMsgPrint,2);

  isis::Property a=std::string("Hallo");
  std::cout << a->toString(true) << std::endl;

  isis::Property b=a;
  std::cout << b->toString(true) << std::endl;

  b=5.2;
  std::cout << b->toString(true) << std::endl;

  isis::PropMap map,cont;
  map["Test1"]=6.4;
  map["Test2"]=6;
  map["Test3"]=std::string("Hallo");

  cont["Prop"]=map;
  std::cout << cont["Prop"]->toString() << std::endl;

  std::string x=map["Test3"];
  std::cout << x << std::endl;

  //will fail at runtime
  int fail=map["Test3"];
  
  return EXIT_SUCCESS;
}
