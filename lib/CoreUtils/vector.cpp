#include "vector.hpp"


namespace isis{ namespace util{
fvector4::fvector4 ( float fourth, float third, float second, float first )
{
	operator[](3)=fourth;
	operator[](2)=third;
	operator[](1)=second;
	operator[](0)=first;
}

fvector4::fvector4(){}

}}
