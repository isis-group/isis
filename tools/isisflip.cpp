#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"

using namespace isis;

int main( int argc, char **argv )
{
	data::IOApplication app( "isisflip", true, true );
	util::Selection swapparam( "x,y,z" );
	swapparam.set("x");
	app.parameters["swap"] = swapparam;
	app.parameters["swap"].needed() = true;
	app.parameters["swap"].setDescription("Swap along the specified axis");
	app.init(argc, argv);
	data::ImageList finImageList;
	BOOST_FOREACH( data::ImageList::const_reference ref, app.images ) {
		std::vector<boost::shared_ptr<data::Chunk> > chList = ref->getChunkList();
		boost::shared_ptr<data::Image> newImage (new data::Image);
		BOOST_FOREACH(std::vector<boost::shared_ptr< data::Chunk> >::const_reference chRef, chList)
		{
			data::Chunk tmpChunk = chRef->cloneToMem(chRef->sizeToVector()[0], chRef->sizeToVector()[1], chRef->sizeToVector()[2], chRef->sizeToVector()[3]);
			if(app.parameters["swap"].toString() == "x") {
					chRef->swapAlong(tmpChunk, 0, true );
			}
			if(app.parameters["swap"].toString() == "y") {
					chRef->swapAlong(tmpChunk, 1, true );
			}
			if(app.parameters["swap"].toString() == "z") {
					chRef->swapAlong(tmpChunk, 2, true );
			}
			newImage->insertChunk(tmpChunk);

		}
		finImageList.push_front(newImage);
	}
	app.autowrite(finImageList);
	return 0;
};
