#include <DataStorage/io_application.hpp>
#include <DataStorage/io_factory.hpp>
#include <boost/foreach.hpp>


using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isis data converter", true, true );
	app.init( argc, argv ); // will exit if there is a problem

	app.autowrite( app.images );
	return EXIT_SUCCESS;
}
