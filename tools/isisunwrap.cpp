#include <isis/core/io_application.hpp>

using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isis unwrap");
	
	app.parameters["dim"]=util::Selection("row,column,slice,time","time");
// 	app.parameters["pos"]=0;
// 	app.parameters["pos"].needed()=false;

	app.init( argc, argv ); // will exit if there is a problem
	
	//set up all the dimensions to traverse trough (aka. all dims except the one we do the statistics accross)
	std::vector<unsigned short> traverse_dims={0,1,2,3};
	unsigned short wrap_dim=app.parameters["dim"].as<util::Selection>()-1;
	traverse_dims.erase(traverse_dims.begin()+wrap_dim);


	for(data::Image &i:app.images){
		auto size = i.getSizeAsVector();
		boost::progress_display progress(size[traverse_dims[0]]*size[traverse_dims[1]]*size[traverse_dims[2]],std::clog << "Unwrapping " << i.identify());

		for(size_t x=0;x<size[traverse_dims[0]];x++)
			for(size_t y=0;y<size[traverse_dims[1]];y++)
				for(size_t z=0;z<size[traverse_dims[2]];z++)
				{
					for(size_t t=0;t<size[wrap_dim]/2;t++){
						std::array<size_t,4> pos,swap;
						pos[traverse_dims[0]]=x;
						pos[traverse_dims[1]]=y;
						pos[traverse_dims[2]]=z;
						pos[wrap_dim]=t;
						swap=pos;
						swap[wrap_dim]=t+size[wrap_dim]/2;
						std::swap(
							i.voxel<int16_t>(pos[0],pos[1],pos[2],pos[3]),
							i.voxel<int16_t>(swap[0],swap[1],swap[2],swap[3])
						);
					}
					++progress;
				}
	}

// 	app.addExample( "-in myFile.nii -out myFile.v", "Simple conversion from a nifti file to a vista file." );
// 	app.addExample(
// 		"-in directory_full_of_dicom_files -out /tmp/S{sequenceNumber}_{sequenceDescription}_e{echoTime}.nii",
// 		"Read all dicom-files (*.ima) from the directory given and write the resulting images as nifti files "
// 		"into /tmp/ generating their names from the sequenceNumber, sequenceDescription and echoTime extracted "
// 		"from the dicom files." );

	app.autowrite( app.images );
	return EXIT_SUCCESS;
}
