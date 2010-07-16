#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"


using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isis data converter", true, true );
	app.parameters["tr"] = u_int16_t();
	app.parameters["tr"] = 0;
	app.parameters["tr"].needed() = false;
	app.parameters["tr"].setDescription( "Repetition time in ms." );

	if ( ! app.init( argc, argv ) ) {
		return EXIT_FAILURE;
	}
	app.autoload();

	if ( app.images.size() == 0 ) {
		LOG( isis::image_io::Runtime, error ) << "No images found. Abort";
	}

	if( app.parameters["tr"] > 0 ) {
		std::cout << "setting tr to " << app.parameters["tr"].toString() << "ms." << std::endl;
		BOOST_FOREACH( data::ImageList::const_reference ref, app.images ) {
			ref->setProperty<u_int16_t>( "repetitionTime", app.parameters["tr"] );
		}
	}

	app.autowrite(app.images);
	return EXIT_SUCCESS;
}
