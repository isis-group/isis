//
// C++ Implementation: ndimensional
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "ndimensional.h"

namespace isis{ namespace data{ namespace _internal{

template<> size_t __dimStride<0>(const size_t dim[])
{
	return 1;
}

template<> size_t __dim2Index<0>(const size_t d[],const size_t dim[])
{
	return d[0]*__dimStride<0>(dim);
}


template<> bool   __rangeCheck<0>(const size_t d[],const size_t dim[])
{
	return d[0]<dim[0];
}

}}}