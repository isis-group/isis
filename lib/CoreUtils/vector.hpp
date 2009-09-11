#include "CoreUtils/log.hpp"
#include "CoreUtils/type.hpp"
#include <cstdarg>

namespace isis{ 
/*! \addtogroup util
 *  Additional documentation for group `mygrp'
 *  @{
 */
namespace util{

template<typename T, size_t SIZE> class FixedVector : public TypePtr<T>{
public:
	FixedVector():TypePtr<T>((T*)calloc(SIZE,sizeof(T)),4){}
	FixedVector(const T src[SIZE]):TypePtr<T>((T*)malloc(SIZE*sizeof(T)),4){
		for(size_t i=0;i<SIZE;i++)
			this->operator[](i)=src[i];
	}
};
}
/** @} */
}