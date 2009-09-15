#include "CoreUtils/log.hpp"
#include "DataStorage/chunk.hpp"
#include "CoreUtils/vector.hpp"

using namespace isis::data;
using namespace isis::util;

struct SpeakingDeleter{
	std::string m_name;
	SpeakingDeleter(std::string name):m_name(name){}
	void operator()(void *p){
		std::cout << "Hello my name is " << m_name << ". I'm your friendly deleter, and I'm freeing now..." << std::endl;
		free(p);
	};
};

int main(){
	ENABLE_LOG(CoreDebug,DefaultMsgPrint,warning);
	ENABLE_LOG(DataDebug,DefaultMsgPrint,warning);
	ENABLE_LOG(DataLog,DefaultMsgPrint,warning);
	
// 	iUtil::DefaultMsgPrint::stopBelow(warning); 
	
	Type<float> float_0_5(0.5);//will implicitly convert the double to float
	std::cout << "float_0_5.toString():" <<  float_0_5.toString() << std::endl;
	
	
	//Type<short> short_0_5((float)float_0_5);Will fail with bad cast
	Type<short> short_0_5((short)float_0_5);//will be ok, because the float-value (which is returned from Type<float>::operator float() ) is implicitely casted to short
	std::cout << "short_0_5.toString():" <<  short_0_5.toString(true) << std::endl;
	
	TypePtr<int> p((int*)calloc(5,sizeof(int)),5,SpeakingDeleter("Rolf"));
	std::cout << "p.toString():" <<  p.toString() << std::endl;

	MemChunk<short> a(1,1,1,50);
	const short x[]={1,2,3,4};
	FixedVector<short,4> v(x);
	
	std::cout << "v.toString():" << v.toString() << std::endl;
	
	a[30]=5;
	std::cout << "a(0,0,0,30):" << a(0,0,0,30) << std::endl;
	std::cout << "a.toString():" << a.toString() << std::endl;
	a(3,0,0,30)=3;//fail (may crash or not)
}