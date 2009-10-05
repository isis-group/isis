//
// C++ Implementation: type_base
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "type_base.hpp"

bool isis::util::_internal::GenericType::isSameType ( const isis::util::_internal::GenericType& second ) const {
	return typeID() == second.typeID();
}
