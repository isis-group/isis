#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include <isis/data/io_factory.hpp>
#include <isis/math/fft.hpp>

namespace isis
{
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( sinus_fft_test )
{
	int xsize=256,ysize=256,zsize=16;
	data::MemChunk<float> sinus(xsize,ysize,zsize,1,true);
	data::MemChunk<float> real(xsize,ysize,zsize,1,true);

	for(int z=0;z<zsize;z++)
		for(int y=0;y<ysize;y++)
			for(int x=0;x<xsize;x++)
			{
				sinus.voxel<float>(x,y,z)=
					(std::sin(x*M_PI*2/xsize)+std::sin(y*M_PI*2/ysize))/4 +
					0.5;
			}
// 	data::IOFactory::write(data::Image(sinus),"/tmp/sinus.nii");


	data::TypedChunk<std::complex< float > > k_space=math::fft_single(sinus,false);

	// fslview doesn't like complex images -- so lets transform it
// 	std::transform(
// 		k_space.begin(),k_space.end(),real.begin(),
// 		[](std::complex< float > v){return v.real();}
// 	);
// 	data::IOFactory::write(data::Image(real),"/tmp/k_space.nii");

	std::pair< util::ValueReference, util::ValueReference > min_max=k_space.getMinMax();

	BOOST_CHECK_EQUAL(
		k_space.voxel<std::complex< float >>(xsize/2,ysize/2,zsize/2).real(),
		min_max.second->as<std::complex< float >>().real()
	);

	BOOST_REQUIRE_CLOSE(
		k_space.voxel<std::complex< float >>(xsize/2,ysize/2,zsize/2).real(),
		0.5 * k_space.getVolume() / sqrt(k_space.getVolume()),
		0.0005
	);

	data::TypedChunk<std::complex< float > > inverse=math::fft_single(k_space,true);
// 	std::transform(
// 		inverse.begin(),inverse.end(),real.begin(),
// 		[](std::complex< float > v){return v.real();}
// 	);
// 	data::IOFactory::write(data::Image(real),"/tmp/inverse.nii");

	for(int z=0;z<zsize;z++)
		for(int y=0;y<ysize;y++)
			for(int x=0;x<xsize;x++)
			{
				const float value=(std::sin(x*M_PI*2/xsize)+std::sin(y*M_PI*2/ysize))/4 + 0.5;
				BOOST_CHECK_CLOSE(inverse.voxel<std::complex< float >>(x,y,z).real()+0.1,value+0.1,0.0005); //+0.1 to stay away from problematic 0
			}
}

}
}
