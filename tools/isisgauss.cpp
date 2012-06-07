#include <DataStorage/io_application.hpp>
#include <Filter/BasicFilter/GaussianFilter.hpp>
#include <Filter/BasicFilter/common.hpp>

using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isis gaussian kernel", true, true );
	app.addExample( "-in myFile.nii -out myFileFiltered.nii -sigma 1.5", "Simple gaussian filter with sigma 1.5 applied to an image." );
	app.addExample( "-in myFile.nii -out myFileFiltered.nii -fwhm 5.25", "Simple gaussian filter with fwhm 5.25 applied to an image." );

	app.parameters["sigma"] = std::numeric_limits< float >::quiet_NaN();
	app.parameters["sigma"].needed() = false;
	app.parameters["sigma"].setDescription( "Standard deviation of the gaussian kernel. Use this or the \"fwhm\" parameter." );

	app.parameters["fwhm"] = std::numeric_limits< float >::quiet_NaN();
	app.parameters["fwhm"].needed() = false;
	app.parameters["fwhm"].setDescription( "Full width half maximum of the gaussian kernel. Use this or the \"sigma\" parameter." );

	app.parameters["physicalSpace"] = true;
	app.parameters["physicalSpace"].needed() = false;
	app.parameters["physicalSpace"].setDescription( "Specifies whether the parameters \"sigma\" or \"fwhm\" are interpreted as mm or voxels." );

	app.init( argc, argv );

	filter::GaussianFilter gaussFilter;
	gaussFilter.setParameters( app.parameters );
	data::Image image = app.fetchImage();


	if( gaussFilter.run( image ) ) {
		app.autowrite( image );
		return EXIT_SUCCESS;
	} else {
		return EXIT_FAILURE;
	}
}