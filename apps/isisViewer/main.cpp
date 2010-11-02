#include "Adapter/qtapplication.hpp"
#include "isisViewer.hpp"

#include "common.hpp"

int main( int argc, char *argv[] )
{
	isis::util::Selection dbg_levels( "error,warning,info,verbose_info" );
	dbg_levels.set( "warning" );

	isis::qt4::QtApplication app( "isisViewer" );
	app.parameters["in"] = isis::util::slist();
	app.parameters["in"].needed() = false;
	app.parameters["in"].setDescription("The input image file list.");
	app.parameters["dViewer"] = dbg_levels;
	app.parameters["dViewer"].setDescription( "Debugging level for the Viewer module" );
	app.parameters["dViewer"].hidden()=true;

	app.init( argc, argv );

	app.setLog<isis::ViewerLog>( app.getLLMap()[app.parameters["dViewer"]->as<isis::util::Selection>()] );
	app.setLog<isis::ViewerDebug>( app.getLLMap()[app.parameters["dViewer"]->as<isis::util::Selection>()] );

	isis::util::slist fileList = app.parameters["in"];
	isis::viewer::isisViewer isisViewerWindow( fileList );
	isisViewerWindow.show();
	return app.getQApplication().exec();
}
