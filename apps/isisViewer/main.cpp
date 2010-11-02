#include "Adapter/qtapplication.hpp"
#include "isisViewer.hpp"


int main( int argc, char *argv[] )
{
	isis::qt4::QtApplication app( "isisViewer" );
	app.parameters["in"] = isis::util::slist();
	app.parameters["in"].needed() = false;
	app.parameters["in"].setDescription("The input image file list.");
	app.init( argc, argv );
	isis::util::slist fileList = app.parameters["in"];
	isis::viewer::isisViewer isisViewerWindow( fileList );
	isisViewerWindow.show();
	return app.getQApplication().exec();
}
