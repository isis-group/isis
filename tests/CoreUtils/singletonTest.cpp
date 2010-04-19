#include "CoreUtils/singletons.hpp"
#include <iostream>


namespace isis
{
namespace test
{

template<int NUMBER> class SingleTest
{
public:
	SingleTest() {
		std::cout << "Creating SingleTest<" << NUMBER << ">" << std::endl;
	}
	~SingleTest() {
		std::cout << "Deleting SingleTest<" << NUMBER << ">" << std::endl;
	}
};
}
}
using namespace isis::util;
using namespace isis::test;
int main()
{
	SingleTest<1> &s1 = Singletons::get<SingleTest<1>, 10>();

	if ( &s1 != &Singletons::get<SingleTest<1>, 0>() )
		std::cerr << "Second request for SingleTest<1> differs" << std::endl;

	if ( ( void* )&s1 == ( void* )&Singletons::get<SingleTest<2>, 5>() )
		std::cerr << "request for SingleTest<2> gets Singleton1" << std::endl;

	if ( ( void* )&s1 == ( void* )&Singletons::get<SingleTest<3>, 5>() ) // this should be deleted before SingleTest<2>
		std::cerr << "request for SingleTest<3> gets Singleton1" << std::endl;
}
