#include "CoreUtils/singletons.hpp"
#include <iostream>

using namespace isis::util;

class SingleTest1:public _internal::Singleton{
public:
	SingleTest1(){
		std::cout << "Creating SingleTest1" << std::endl;
	}
	~SingleTest1(){
		std::cout << "Deleting SingleTest1" << std::endl;
	}
};
class SingleTest2:public _internal::Singleton{
public:
	SingleTest2(){
		std::cout << "Creating SingleTest2" << std::endl;
	}
	~SingleTest2(){
		std::cout << "Deleting SingleTest2" << std::endl;
	}
};

int main(){
	SingleTest1 &s1= Singletons::get<SingleTest1,10>();
	if(&s1!= &Singletons::get<SingleTest1,0>())
		std::cout << "Second request differs" << std::endl;
	if((void*)&s1== (void*)&Singletons::get<SingleTest2,5>())
		std::cout << "request for Singleton2 gets Singleton1" << std::endl;
}