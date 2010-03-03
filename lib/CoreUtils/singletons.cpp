#include "singletons.hpp"
#include <boost/foreach.hpp>

namespace isis{ namespace util{

	Singletons& Singletons::getMaster()
{
	static Singletons me;
	return me;
}
Singletons::~Singletons()
{
	BOOST_FOREACH(prioMap::const_reference ref,map){
		delete ref.second;
	}
}

}}