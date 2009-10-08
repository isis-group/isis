//
// C++ Implementation: vector
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

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
