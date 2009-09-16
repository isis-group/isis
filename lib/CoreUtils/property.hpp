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

class Property:public boost::shared_ptr<_internal::TypeBase>{
	bool m_needed;
public:
	template<typename T> Property(const T& ref):boost::shared_ptr <_internal::TypeBase >(new Type<T>(ref)),m_needed(false){ }
	Property():m_needed(false){ }
	template<typename T> operator T()const{
		const _internal::TypeBase *dummy=get();
		const Type<T> ret=dummy->cast_to_Type<T>();
		return (T)ret;
	}
	bool empty(){
		return get()!=NULL;
	}
	bool &needed(){
		return m_needed;
	}
};

}
/** @} */
}

#endif


