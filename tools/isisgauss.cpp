#include <DataStorage/io_application.hpp>
#include <Filter/BasicFilter/GaussianFilter.hpp>
#include <Filter/BasicFilter/common.hpp>

using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isis data converter", true, true );
	app.addExample( "-in myFile.nii -out myFileFiltered.nii -sigma 1.5", "Simple gaussian filter with sigma 1.5 applied to an image." );
	app.addExample( "-in myFile.nii -out myFileFiltered.nii -fwhm 1.5", "Simple gaussian filter with fwhm 5.25 applied to an image." );
	app.parameters["sigma"] = std::numeric_limits< float >::quiet_NaN();
	app.parameters["fwhm"] = std::numeric_limits< float >::quiet_NaN();
	app.parameters["sigma"].needed() = false;
	app.parameters["fwhm"].needed() = false;
	app.init( argc, argv );

	filter::GaussianFilter gaussFilter;
	gaussFilter.parameters = app.parameters;
	data::Image image = app.fetchImage();

	if( gaussFilter.run( image ) ) {
		app.autowrite( image );
	} else {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}