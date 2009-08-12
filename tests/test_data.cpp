#include "CoreUtils/log.hpp"
#include <DataStorage/chunk.hpp>

using namespace iUtil;

int main(){
	ENABLE_LOG(CoreLog,DefaultMsgPrint,info);
	
	iData::Chunk<short> a(new short[50]);
}