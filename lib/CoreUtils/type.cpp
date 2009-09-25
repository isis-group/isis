#include "type.hpp"

namespace isis{ namespace util{ namespace _internal{

TypeReference::TypeReference(TypeBase *t):boost::shared_ptr <_internal::TypeBase >(t){}
TypeReference::TypeReference(){}
bool TypeReference::empty(){
	return get()!=NULL;
}

}}}

