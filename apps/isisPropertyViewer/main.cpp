#include <QApplication>
#include "isisPropertyViewer.h"

int main( int argc, char *argv[] )
{
	QApplication app( argc, argv );
	isisPropertyViewer isisPropertyViewerWindow;
	isisPropertyViewerWindow.show();
	return app.exec();
}