#include <DataStorage/io_application.hpp>

using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isis data converter", true, true );
	app.init( argc, argv ); // will exit if there is a problem

	app.autowrite( app.images );
	return EXIT_SUCCESS;
}
