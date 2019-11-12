#include <isis/core/io_application.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/progress.hpp>


struct StatLog {static const char *name() {return "Raw";}; enum {use = _ENABLE_LOG};};
struct StatDebug {static const char *name() {return "RawDebug";}; enum {use = _ENABLE_DEBUG};};

using namespace boost::accumulators;
using namespace isis;

int main( int argc, char *argv[] )
{
	data::IOApplication app("isisstat");
	
	app.parameters["dim"]=util::Selection("row,column,slice,time","time");

	app.addExample( "-in my_file.nii -out /tmp/raw.file -repn u8bit", "Write the image data of a nifti file in a u8bit raw file" );
	app.addExample( "-in raw.file -read_repn s16bit -out new_image.nii -rawdims 384 384 12 -offset 500", "Read 384*384*12 s16bit blocks from a raw file skipping 500 bytes and store them as a nifti image." );

	app.init( argc, argv, true ); // if there is a problem, we just get no images and exit cleanly
	std::list<data::Image> output;

	while(app.images.size()){
		//extract image
		auto img=app.fetchImageAs<float>(false);
		auto size = img.getSizeAsVector();
		data::MemChunk<float> dest(size[0],size[1],size[2]);
		static_cast<util::PropertyMap&>(dest)=img;
		
		boost::progress_display progress(size[0]*size[1]*size[2],std::clog << "Computing sum of squares for " << img.identify());
		
		//set up all the dimensions to traverse trough (aka. all dims except the one we do the statistics accross)
		std::vector<unsigned short> traverse_dims={0,1,2,3};
		unsigned short stat_dim=app.parameters["dim"].as<unsigned short>()-1;
		traverse_dims.erase(traverse_dims.begin()+stat_dim);
		
		for(size_t x=0;x<size[traverse_dims[0]];x++)
			for(size_t y=0;y<size[traverse_dims[1]];y++)
				for(size_t z=0;z<size[traverse_dims[2]];z++)
				{
					accumulator_set<float, stats<tag::variance> > acc;
					for(size_t t=0;t<size[stat_dim];t++){
						std::array<size_t,4> pos;
						pos[traverse_dims[0]]=x;
						pos[traverse_dims[1]]=y;
						pos[traverse_dims[2]]=z;
						pos[stat_dim]=t;
						acc(img.voxel<float>(pos[0],pos[1],pos[2],pos[3]));
					}
					dest.voxel<float>(x,y,z)=std::sqrt(variance(acc));
					++progress;
				}
					
		output.push_back(data::Image(dest));
	}
	app.autowrite(output);

	return 0;
}
