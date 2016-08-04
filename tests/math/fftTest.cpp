#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include "data/io_factory.hpp"
#include "../math/gsl/fft.hpp"

namespace isis
{
	namespace math{
		data::MemChunk<std::complex< float >> fft( data::MemChunk<std::complex< float >> src, bool inverse=false);
	}
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( sinus_fft_test )
{
	int xsize=M_PI*2*50,ysize=M_PI*2*50,zsize=10;
	data::MemChunk<float> sinus(xsize,ysize,zsize,1,true),real(xsize,ysize,zsize,1,true);

	for(int z=0;z<zsize;z++)
		for(int y=0;y<ysize;y++)
			for(int x=0;x<xsize;x++)
			{
				sinus.voxel<float>(x,y,z)=
					(std::sin(x/50.)+std::sin(y/50.))/4 +
					0.5;
			}


	data::MemChunk<std::complex< double > > k_space=math::fft(sinus,false);
	std::pair< util::ValueReference, util::ValueReference > min_max=k_space.getMinMax();

	BOOST_CHECK_EQUAL(
		k_space.voxel<std::complex< double >>(xsize/2,ysize/2,zsize/2).real(),
		min_max.second->as<std::complex< double >>().real()
	);

	BOOST_CHECK_CLOSE(
		k_space.voxel<std::complex< double >>(xsize/2,ysize/2,zsize/2).real(),
		1,
		0.001
	);
	

	data::MemChunk<std::complex< double > > inverse=math::fft(k_space,true);
	
	for(int z=0;z<zsize;z++)
		for(int y=0;y<ysize;y++)
			for(int x=0;x<xsize;x++)
			{
				BOOST_CHECK_CLOSE(
					inverse.voxel<std::complex< double >>(x,y,z).real(),
					(std::sin(x/50.)+std::sin(y/50.))/4 + 0.5,
					0.00001
				);
			}

// 	data::IOFactory::write(data::Image(sinus),"/tmp/sinus.nii");

// 	std::transform(
// 		k_space.begin(),k_space.end(),real.begin(),
// 		[](std::complex< double > v){return v.real();}
// 	);
// 	data::IOFactory::write(data::Image(real),"/tmp/fft.nii");

// 	std::transform(
// 		inverse.begin(),inverse.end(),real.begin(),
// 		[](std::complex< double > v){return v.real();}
// 	);
// 	data::IOFactory::write(data::Image(real),"/tmp/inverse.nii");

}

}
}
