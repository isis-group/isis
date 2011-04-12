#include "Adapter/qtapplication.hpp"
#include "MainWindow.hpp"
#include "QViewerCore.hpp"
#include <iostream>
#include <GL/gl.h>
#include <DataStorage/io_factory.hpp>
#include <DataStorage/image.hpp>
#include <CoreUtils/log.hpp>

#include "common.hpp"

int main( int argc, char *argv[] )
{
	isis::util::Selection dbg_levels( "error,warning,info,verbose_info" );
	dbg_levels.set( "warning" );
	isis::qt4::QtApplication app( "isisViewer" );
	app.parameters["in"] = isis::util::slist();
	app.parameters["in"].needed() = false;
	app.parameters["in"].setDescription( "The input image file list." );
	app.parameters["dViewer"] = dbg_levels;
	app.parameters["dViewer"].setDescription( "Debugging level for the Viewer module" );
	app.parameters["dViewer"].hidden() = true;
	app.parameters["dViewer"].needed() = false;
	app.init( argc, argv );
	app.setLog<isis::ViewerLog>( app.getLLMap()[app.parameters["dViewer"]->as<isis::util::Selection>()] );
	app.setLog<isis::ViewerDebug>( app.getLLMap()[app.parameters["dViewer"]->as<isis::util::Selection>()] );
	isis::util::slist fileList = app.parameters["in"];
	std::list<isis::data::Image> imgList;
	BOOST_FOREACH ( isis::util::slist::const_reference filename, fileList )
	{
		std::list< isis::data::Image > tmpList = isis::data::IOFactory::load( filename );
		BOOST_FOREACH( std::list< isis::data::Image >::const_reference imgRef, tmpList )
		{
			imgList.push_back( imgRef );
		}
	}
	isis::viewer::QViewerCore *core = new isis::viewer::QViewerCore;
	isis::viewer::MainWindow isisViewerMainWindow( core );
	core->setImageList( imgList, fileList );
	isisViewerMainWindow.show();
	return app.getQApplication().exec();
}
