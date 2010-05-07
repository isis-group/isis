#include <QApplication>
#include "isisPropertyViewer.hpp"

#include "CoreUtils/application.hpp"


int main( int argc, char *argv[] )
{
	isis::util::QtApplication app( "isisPropertyViewer", argc, argv );
	app.parameters["in"] = isis::util::slist();
	app.parameters["in"].needed() = false;
	app.parameters["in"].setDescription( "input file list." );
	app.init( argc, argv );
	isis::util::slist fileList = app.parameters["in"];
	isisPropertyViewer isisPropertyViewerWindow( fileList );
	isisPropertyViewerWindow.show();
	return app.exec();
}