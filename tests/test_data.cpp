#include "CoreUtils/log.hpp"
#include "DataStorage/chunk.hpp"
#include "CoreUtils/vector.hpp"

using namespace isis::data;
using namespace isis::util;

int main(){
	ENABLE_LOG(CoreDebug,DefaultMsgPrint,info);
	ENABLE_LOG(DataDebug,DefaultMsgPrint,info);
	ENABLE_LOG(DataLog,DefaultMsgPrint,info);
	
// 	iUtil::DefaultMsgPrint::stopBelow(iUtil::warning);
	
	MemChunk<short> a(1,1,1,50);
	const short x[]={1,2,3,4};
	FixedVector<short,4> v(x);
	
	std::cout << v.toString() << std::endl;
	
	a[30]=5;
	std::cout << a(0,0,0,30) << std::endl;
	std::cout << a.toString() << std::endl;
	a(3,0,0,30)=3;//fail
}