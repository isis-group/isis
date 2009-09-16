#include "CoreUtils/log.hpp"
#include "CoreUtils/type.hpp"
#include <cstdarg>

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

template<typename TYPE, size_t SIZE> class FixedVector : public TypePtr<TYPE>{
public:
	FixedVector():TypePtr<TYPE>((TYPE*)calloc(SIZE,sizeof(TYPE)),4){}
	FixedVector(const TYPE src[SIZE]):TypePtr<TYPE>((TYPE*)malloc(SIZE*sizeof(TYPE)),4){
		for(size_t i=0;i<SIZE;i++)
			this->operator[](i)=src[i];
	}
};
}
/** @} */
}