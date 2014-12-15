#include "DataStorage/io_factory.hpp"
#include <fstream>
#include "DataStorage/io_application.hpp"
#include <iomanip>

using namespace isis;

int main( int argc, char *argv[] )
{
	data::IOApplication app( "isis data dumper", true, false );
	// ********* Parameters *********
	app.parameters["chunks"] = false;
	app.parameters["chunks"].needed() = false;
	app.parameters["chunks"].setDescription( "print detailed data about the subsections (chunks) for each image" );

	app.addExample( "-in file.nii", "Print all metadata of the image in a nifti file." );
	app.addExample( "-in directory_full_of_dicom_files -rf ima",
					"Print all metadata of all images red from a directory and enforce the file format \"ima\" (DICOM) when reading the files in that directory." );

	app.init( argc, argv, false ); // if there is a problem, we just get no images and exit cleanly
	unsigned short count1 = 0;

	if( app.images.size() > 1 )
		std::cout << "There are " << app.images.size() << " Images in the dataset." << std::endl;

	const unsigned short imageDigits = std::log10( app.images.size() ) + 1;
	std::cout.fill( '0' );
	for( data::Image & ref :  app.images ) {
		std::cout << "======Image #" << std::setw( imageDigits )  << ++count1 << std::setw( 0 ) << " " << ref.getSizeAsString() << " (" << ref.identify( false ) << ") ======" << std::endl;
		ref.print( std::cout, true );
		int count2 = 0;

		if( app.parameters["chunks"] ) {
			std::vector<data::Chunk> chunks = ref.copyChunksToVector( false );
			const unsigned short chunkDigits = std::log10( chunks.size() ) + 1;
			for( const data::Chunk & c :  chunks ) {
				std::cout
						<< "======Image #" << std::setw( imageDigits )  << count1 << std::setw( 0 )
						<< "==Chunk #" << std::setw( chunkDigits )  << ++count2 << std::setw( 0 ) << " "
						<< c.getSizeAsString() << c.getTypeName() << "======Metadata======" << std::endl;
				c.print( std::cout, true );
			}
		}
	}
	return 0;
}
