#include "singletons.hpp"
#include <boost/foreach.hpp>

namespace isis
{
namespace util
{

Singletons &Singletons::getMaster()
{
	static Singletons me;
	return me;
}
Singletons::~Singletons()
{
	while ( !map.empty() ) {
		delete map.begin()->second;
		map.erase( map.begin() );
	}
}
Singletons::Singletons() {}
Singletons::SingletonBase::~SingletonBase() {}

}
}
