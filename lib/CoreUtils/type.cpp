#include "type.hpp"

bool isis::util::_internal::GenericType::isSameType ( const isis::util::_internal::GenericType& second ) const {
	return typeID() == second.typeID();
}
