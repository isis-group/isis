#ifndef ISISPROPERTY_HPP
#define ISISPROPERTY_HPP

#include <boost/shared_ptr.hpp>
#include <map>
#include "type.hpp"
#include "log.hpp"

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

/**
common property class

	@author Enrico Reimer
*/

class Property:public _internal::TypeContainer{
	bool m_needed;
public:
	template<typename T> Property(const T& ref):_internal::TypeContainer(new Type<T>(ref)),m_needed(false){ }
	Property():m_needed(false){ }
	template<typename T> operator T()const{
		const _internal::TypeBase *dummy=get();
		const Type<T> ret=dummy->cast_to_Type<T>();
		return (T)ret;
	}
	bool &needed(){
		return m_needed;
	}
};

}
/** @} */
}

#endif


