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
	
	using namespace isis::viewer;
	isis::util::Selection dbg_levels( "error,warning,info,verbose_info" );
	dbg_levels.set( "warning" );
	
	isis::util::Selection image_types( "anatomical,zmap" );
	image_types.set( "anatomical" );
	isis::qt4::QtApplication app( "isisViewer" );
	app.parameters["in"] = isis::util::slist();
	app.parameters["in"].needed() = false;
	app.parameters["in"].setDescription( "The input image file list." );
	app.parameters["z"] = isis::util::slist();
	app.parameters["z"].needed() = false;
	app.parameters["z"].setDescription( "The input image file list interpreted as zmaps. " );
	app.parameters["type"] = image_types;
	app.parameters["type"].needed() = false;
	app.parameters["type"].setDescription( "The type as what the image should be interpreted." );
	app.parameters["dViewer"] = dbg_levels;
	app.parameters["dViewer"].setDescription( "Debugging level for the Viewer module" );
	app.parameters["dViewer"].hidden() = true;
	app.parameters["dViewer"].needed() = false;
	app.init( argc, argv );
	app.setLog<isis::ViewerLog>( app.getLLMap()[app.parameters["dViewer"]->as<isis::util::Selection>()] );
	app.setLog<isis::ViewerDebug>( app.getLLMap()[app.parameters["dViewer"]->as<isis::util::Selection>()] );
	isis::util::slist fileList = app.parameters["in"];
	isis::util::slist zmapFileList = app.parameters["z"];
	std::list<isis::data::Image> imgList;
	std::list<isis::data::Image> zImgList;
	//load the anatomical images
	BOOST_FOREACH ( isis::util::slist::const_reference filename, fileList )
	{
		std::list< isis::data::Image > tmpList = isis::data::IOFactory::load( filename );
		BOOST_FOREACH( std::list< isis::data::Image >::const_reference imgRef, tmpList )
		{
			imgList.push_back( imgRef );
		}
	}
	//load the zmap images
	BOOST_FOREACH ( isis::util::slist::const_reference filename, zmapFileList )
	{
		std::list< isis::data::Image > tmpList = isis::data::IOFactory::load( filename );
		BOOST_FOREACH( std::list< isis::data::Image >::const_reference imgRef, tmpList )
		{
			zImgList.push_back( imgRef );
		}
	}
	isis::viewer::QViewerCore *core = new isis::viewer::QViewerCore;
	isis::viewer::MainWindow isisViewerMainWindow( core );
	
	if(app.parameters["z"].isSet()) {
		core->addImageList( zImgList, ImageHolder::z_map, zmapFileList );
	}
	
	if(app.parameters["type"].toString() == "anatomical" && app.parameters["in"].isSet()) {
		core->addImageList( imgList, ImageHolder::anatomical_image, fileList );
	} else if ( app.parameters["type"].toString() == "zmap" && app.parameters["in"].isSet()) {
		core->addImageList( imgList, ImageHolder::z_map, fileList );
	}
	isisViewerMainWindow.show();
	return app.getQApplication().exec();
}
