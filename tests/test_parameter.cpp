

#include "CoreUtils/application.hpp"

using namespace isis;

int main( int argc, char **argv )
{
	util::Application app( "isis program parameter example" );
	app.parameters["dump"] = std::string( "/tmp/delme" );
	app.parameters["dump"].isNeeded() = false;
	app.parameters["offset"] = util::dlist();
	util::Selection selectionTest( "select1,select2" );
	selectionTest.set( "select1" );
	app.parameters["select"] = selectionTest;
	app.parameters["select"].setDescription( "Selection type test." );
	app.parameters["select"].isNeeded() = true;
	app.init( argc, argv );
	std::cout << "Selection: " << app.parameters["select"].toString() << std::endl;
	std::cout << "The dump file: " << app.parameters["dump"].toString() << std::endl;
	std::cout << "The offset as parameter:" << app.parameters["offset"] << std::endl; //prints as ProgParameter
	std::cout << "The offset as Value:" << app.parameters["offset"].toString( true ) << std::endl; //prints as string from TypeBase::toString
}