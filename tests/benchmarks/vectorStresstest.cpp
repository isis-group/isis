#include <isis/core/matrix.hpp>
#include <boost/timer.hpp>

using namespace isis;

int main()
{
	boost::timer timer;

	util::vector4<float> v1{1, 1, 0, 0};

	util::Matrix4x4<float> test{
		1 / sqrt( 2 ), -1 / sqrt( 2 ), 0, 0,
		1 / sqrt( 2 ),  1 / sqrt( 2 ), 0, 0
	};
	const float len = util::len(v1);

	timer.restart();

	for( size_t i = 0; i < 99999999; i++ )
		if( (test * v1)[1] != len )
			std::cout << "Error, result is wrong " << test * v1 << std::endl;


	std::cout << timer.elapsed() << " sec for matrix by vector mult" << std::endl;

	timer.restart();

	for( size_t i = 0; i < 99999999; i++ )
		if( v1 *v1 != v1 )
			std::cout << "Error, result is wrong " << v1 *v1 << std::endl;


	std::cout << timer.elapsed() << " sec for vector by vector mult" << std::endl;

}
