#include "singletons.hpp"

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
		map.begin()->second();
		map.erase( map.begin() );
	}
}
Singletons::Singletons() {}

}
}
