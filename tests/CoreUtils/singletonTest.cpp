#include "CoreUtils/singletons.hpp"
#include <iostream>

using namespace isis::util;

template<int NUMBER> class SingleTest{
public:
	SingleTest(){
		std::cout << "Creating SingleTest<" << NUMBER << ">" << std::endl;
	}
	~SingleTest(){
		std::cout << "Deleting SingleTest<" << NUMBER << ">" << std::endl;
	}
};

int main(){
	SingleTest<1> &s1= Singletons::get<SingleTest<1>,10>();
	if(&s1!= &Singletons::get<SingleTest<1>,0>())
		std::cout << "Second request for SingleTest<1> differs" << std::endl;

	if((void*)&s1== (void*)&Singletons::get<SingleTest<2>,5>())
		std::cout << "request for SingleTest<2> gets Singleton1" << std::endl;
	if((void*)&s1== (void*)&Singletons::get<SingleTest<3>,5>()) // this should be deleted before SingleTest<2> 
		std::cout << "request for SingleTest<3> gets Singleton1" << std::endl;
}
