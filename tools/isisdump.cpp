#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include <fstream>
#include "DataStorage/io_application.hpp"

using namespace isis;

int main( int argc, char *argv[] )
{
	data::IOApplication app( "isis data dumper", true, false );
	app.parameters["chunks"] = false;
	app.parameters["chunks"].needed() = false;
	app.parameters["chunks"].setDescription( "print also data of the chunks" );

	if ( not app.init( argc, argv ) )
		return 1;

	unsigned short count1 = 0, count2 = 0;
	std::cout << "Got " << app.images.size() << " Images" << std::endl;
	std::ofstream dump;

	BOOST_FOREACH( data::ImageList::const_reference ref, app.images ) {
		std::cout << "======Image #" << ++count1 << ref->sizeToString() << "======Metadata======" << std::endl;
		ref->print( std::cout, true );
		if(app.parameters["chunks"]){
			for ( data::Image::ChunkIterator c = ref->chunksBegin(); c != ref->chunksEnd(); c++ ) {
				std::cout << "======Image #" << count1 << "==Chunk #" << ++count2 << c->sizeToString() << c->typeName() << "======Metadata======" << std::endl;
				c->print( std::cout, true );

				if ( dump.is_open() ) {
					dump << "======Image #" << count1 << "==Chunk #" << ++count2 << c->sizeToString() << c->typeName() << "======Voxel Data======" << std::endl;
					dump << c->getTypePtrBase().toString() << std::endl;
				}
			}
		}
	}
	return 0;
}
