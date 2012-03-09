#include "DataStorage/typeptr.hpp"
#include <boost/timer.hpp>

using namespace isis;

template<typename T> void testMinMax( size_t size )
{
	boost::timer timer;
	data::ValueArray<T> array( ( T * )malloc( size ), size / sizeof( T ) );

	timer.restart();
	array.getMinMax();
	std::cout
			<< "found min/max of " << size / 1024 / 1024 << "MB of " << data::ValueArray<T>::staticName()
			<< " in " << timer.elapsed() << " seconds " << std::endl;

}
int main()
{
	data::enableLog<util::DefaultMsgPrint>( verbose_info ); //set to "verbose_info" to see which alg is used

	testMinMax< int8_t>( 1024 * 1024 * 512 );
	testMinMax<int16_t>( 1024 * 1024 * 512 );
	testMinMax<int32_t>( 1024 * 1024 * 512 );

	testMinMax< uint8_t>( 1024 * 1024 * 512 );
	testMinMax<uint16_t>( 1024 * 1024 * 512 );
	testMinMax<uint32_t>( 1024 * 1024 * 512 );

	testMinMax< float>( 1024 * 1024 * 512 );
	testMinMax<double>( 1024 * 1024 * 512 );
	return 0;
}
