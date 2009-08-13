#include "CoreUtils/log.hpp"
#include <DataStorage/chunk.hpp>

using namespace iData;
using namespace iUtil;

int main(){
	ENABLE_LOG(CoreDebug,iUtil::DefaultMsgPrint,iUtil::info);
	ENABLE_LOG(DataDebug,iUtil::DefaultMsgPrint,iUtil::info);
	ENABLE_LOG(DataLog,iUtil::DefaultMsgPrint,iUtil::info);
	
// 	iUtil::DefaultMsgPrint::stopBelow(iUtil::warning);
	
	MemChunk<short> a(1,1,1,50);
	a[30]=5;
	std::cout << a(0,0,0,30) << std::endl;
	a(3,0,0,30)=3;//fail
}