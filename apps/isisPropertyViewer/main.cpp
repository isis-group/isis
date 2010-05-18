#include <QApplication>
#include "isisPropertyViewer.hpp"

#include "CoreUtils/application.hpp"
#include <ExternalLibraryAdapter/qt4/qtapplication.hpp>


int main( int argc, char *argv[] )
{
	std::cout << "Core Version: " << isis::util::Application::getCoreVersion() << std::endl;
	isis::qt4::QtApplication app( "isisPropertyViewer" );
	app.parameters["in"] = isis::util::slist();
	app.parameters["in"].needed() = false;
	app.parameters["in"].setDescription( "Input file list." );
	app.init( argc, argv );
	isis::util::slist fileList = app.parameters["in"];
	isisPropertyViewer isisPropertyViewerWindow( fileList );
	isisPropertyViewerWindow.show();
	return app.getQApplication().exec();
}