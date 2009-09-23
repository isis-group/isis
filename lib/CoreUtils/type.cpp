#include "type.hpp"

namespace isis{ namespace util{ namespace _internal{

TypeContainer::TypeContainer(TypeBase *t):boost::shared_ptr <_internal::TypeBase >(t){}
TypeContainer::TypeContainer(){}
bool TypeContainer::empty(){
	return get()!=NULL;
}

}}}

