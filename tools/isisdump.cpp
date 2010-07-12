#include "DataStorage/io_factory.hpp"
#include <boost/foreach.hpp>
#include <fstream>
#include "DataStorage/io_application.hpp"
#include <iomanip>

using namespace isis;

int main( int argc, char *argv[] )
{
	data::IOApplication app( "isis data dumper", true, false );
	// ********* Parameters *********
	app.parameters["chunks"] = false;
	app.parameters["chunks"].needed() = false;
	app.parameters["chunks"].setDescription( "print detailed data about the chunks" );

	app.init( argc, argv ); // will exit if there is a problem

	unsigned short count1 = 0;
	std::cout << "Got " << app.images.size() << " Images" << std::endl;
	const unsigned short imageDigits=std::log10(app.images.size())+1;
	std::cout.fill('0');

	BOOST_FOREACH( data::ImageList::const_reference ref, app.images ) {
		std::cout << "======Image #" << std::setw(imageDigits)  << ++count1 << std::setw(0) << ref->sizeToString() << "======Metadata======" << std::endl;
		ref->print( std::cout, true );
		int count2=0;
		if(app.parameters["chunks"]){
			const unsigned short chunkDigits=std::log10(std::distance(ref->chunksBegin(),ref->chunksEnd()))+1;
			for ( data::Image::ChunkIterator c = ref->chunksBegin(); c != ref->chunksEnd(); c++ ) {
				std::cout << "======Image #" << std::setw(imageDigits)  << count1 << std::setw(0) << "==Chunk #" << std::setw(chunkDigits)  << ++count2 << std::setw(0) << c->sizeToString() << c->typeName() << "======Metadata======" << std::endl;
				c->print( std::cout, true );
			}
		}
	}
	return 0;
}
