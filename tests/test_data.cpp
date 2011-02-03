#include "CoreUtils/log.hpp"
#include "DataStorage/chunk.hpp"
#include "CoreUtils/vector.hpp"
#include "DataStorage/io_factory.hpp"

using namespace isis::data;
using namespace isis::util;

namespace isis
{
namespace test
{
struct SpeakingDeleter {
	std::string m_name;
	SpeakingDeleter( std::string name ): m_name( name ) {}
	void operator()( void *p ) {
		std::cout << "Hello my name is " << m_name << ". I'm your friendly deleter, and I'm freeing now..." << std::endl;
		free( p );
	};
};
}
}//namespace test isis

int main()
{
	ENABLE_LOG( CoreDebug, DefaultMsgPrint, info );
	ENABLE_LOG( CoreLog, DefaultMsgPrint, info );
	ENABLE_LOG( DataDebug, DefaultMsgPrint, info );
	ENABLE_LOG( DataLog, DefaultMsgPrint, info );
	IOFactory::get();//get the IO Factory
	IOFactory::get().print_formats( std::cout );//do it again - it should not cause reinit
	IOFactory::get().load( "test.null", "" );
	IOFactory::get().load( "test.null", "" );
	IOFactory::get().load( "test.null", "dia1" );
	IOFactory::get().load( "test.null", "dia2" );
	IOFactory::get().load( "test.xxx", "" );//returns that plugin is missing
	//  iUtil::DefaultMsgPrint::stopBelow(warning);
	Type<float> float_0_5( 0.5 );//will implicitly convert the double to float
	std::cout << "float_0_5.toString():" <<  float_0_5.toString() << std::endl;
	::isis::util::_internal::TypeBase *mephisto = new Type<std::string>( "666" );
	int devil = mephisto->as<int>();
	std::string lucifer = mephisto->as<std::string>();

	try {
		Type<short> short_0_5( ( float )float_0_5 ); // Will throw exception because conversion float->int is "bad"
		//Type<short> short_0_5((short)float_0_5);//will be ok, because the float-value (which is returned from Type<float>::operator float() ) is implicitely casted to short
		std::cout << "short_0_5.toString():" <<  short_0_5.toString( true ) << std::endl;
	} catch ( boost::bad_lexical_cast & ) {
		std::cout << "Whoops " << std::endl;
	}

	Type<int> i( 5 );
	float f_ = i;
	std::cout << "Type<int>(5) is " << i.toString( true ) << " float from that is " << f_ << std::endl;
	Type<float> f( 5.4 );
	int i_ = f;//this wont throw an exception because it only does an implizit conversions from Type<float>=>float and float=>int
	std::cout << "Type<float>(5.4) is " << f.toString( true ) << " int from that is " << i_ << std::endl;
	TypePtr<int> p( ( int * )calloc( 5, sizeof( int ) ), 5, isis::test::SpeakingDeleter( "Rolf" ) );
	std::cout << "p.toString():" <<  p.toString() << std::endl;
	const short x[] = {1, 2, 3, 4};
	FixedVector<short, 4> v( x );
	//  std::cout << "v.toString():" << v.toString() << std::endl;
	std::list<data::Chunk> list;
	{
		MemChunk<short> a( 1, 1, 1, 10 );
		list.push_back( a );
		a.asTypePtr<short>()[5] = 5;
		std::cout << "a.voxel<short>(0,0,0,5):" << a.voxel<short>( 0, 0, 0, 5 ) << std::endl;
		a.voxel<short>( 3, 0, 0, 5 ) = 3;//fail (may crash or not)
	}
	Chunk cp = *list.begin();
	std::cout << "cp.voxel(0,0,0,5):" << cp.voxel<short>( 0, 0, 0, 5 ) << std::endl;
	std::cout << "list.begin()->getTypePtr<short>().toString():" << list.begin()->getTypePtr<short>().toString() << std::endl;
}
