#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"


using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app("isis data converter", true, true);

	if (! app.init(argc, argv )) {
		return EXIT_FAILURE;
	}

	if (app.images.size() == 0){
		LOG(isis::image_io::Runtime, error) << "No images found. Abort";
	}

	data::IOFactory::get().write(app.images, app.parameters["out"]->as<std::string>(),"" );


	return EXIT_SUCCESS;
}
