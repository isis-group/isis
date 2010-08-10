#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"

using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isisflip", true, true );
	util::Selection swapparam( "x,y,z" );
	util::Selection what( "image,space,both" );
	swapparam.set( "x" );
	what.set("both");
	app.parameters["swap"] = swapparam;
	app.parameters["swap"].needed() = true;
	app.parameters["swap"].setDescription( "Swap along the specified axis" );

	app.parameters["what"] = what;
	app.parameters["what"].needed() = true;
	app.parameters["what"].setDescription( "What has to be swapped" );

	app.init( argc, argv );
	data::ChunkList finChunkList;
	unsigned int dim=0;
	BOOST_FOREACH( data::ImageList::const_reference ref, app.images ) {
		std::vector<boost::shared_ptr<data::Chunk> > chList = ref->getChunkList();
		boost::shared_ptr<data::Image> newImage ( new data::Image );
		newImage->join(static_cast<util::PropMap>(*ref));
		BOOST_FOREACH( std::vector<boost::shared_ptr< data::Chunk> >::reference chRef, chList ) {
			chRef->join(static_cast<util::PropMap>(*ref), false);
			data::Chunk tmpChunk = *chRef;
			if( app.parameters["swap"].toString() == "x" ) {
				dim = 0;
			}
			if( app.parameters["swap"].toString() == "y" ) {
				dim = 1;
			}
			if( app.parameters["swap"].toString() == "z" ) {
				dim = 2;
			}
			if ( app.parameters["what"].toString() == "both") {
				chRef->swapAlong( tmpChunk, dim, false );
			} else if ( app.parameters["what"].toString() == "image") {
				chRef->swapAlong( tmpChunk, dim, true );
			}
			finChunkList.push_back(boost::shared_ptr<data::Chunk>(new data::Chunk(tmpChunk)));
		}
	}
	data::ImageList finImageList ( finChunkList );
	app.autowrite( finImageList );
	return 0;
};
