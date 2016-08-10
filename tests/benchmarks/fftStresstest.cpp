#include <isis/math/fft.hpp>
#include <boost/timer.hpp>

using namespace isis;

int main()
{

	int xsize=512,ysize=512,zsize=256;
	data::MemChunk<std::complex< float >> sinus(xsize,ysize,zsize,1,true);
	std::cout << "Filling " << sinus.getSizeAsString() << " matrix (" <<  sinus.getVolume()*sinus.getBytesPerVoxel() / 1024 / 1024 << "MB) with a sinus" << std::endl;

	for(int y=0;y<ysize;y++)
		for(int x=0;x<xsize;x++)
		{
			float value = (std::sin(x*M_PI*2/xsize)+std::sin(y*M_PI*2/ysize))/4 +	0.5;
			for(int z=0;z<zsize;z++)
			{
				sinus.voxel<std::complex< float >>(x,y,z)=value;
			}
		}

	boost::timer timer;

	data::Chunk k_space=math::fft(sinus,false);

	std::cout << "fft took " << timer.elapsed() << " seconds" << std::endl;

	std::cout << "Error was " <<
		k_space.getMinMax().second->as<std::complex< float >>().real()-k_space.voxel<std::complex< float >>(xsize/2,ysize/2,zsize/2).real()<< std::endl;

}
