//
// C++ Implementation: propmap
//
// Description:
//
//
// Author:  <Enrico Reimer>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "propmap.hpp"

bool isis::util::_internal::nocase_less::operator() (const std::string& a, const std::string& b) const	{
	return (strcasecmp (a.c_str ( ), b.c_str ( )) < 0);
}
