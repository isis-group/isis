#include <iostream>
#include <isis/adapter/qt5/common.hpp>
#include <isis/adapter/qt5/qtapplication.hpp>
#include <isis/adapter/qt5/simpleimageview.hpp>

using namespace isis::qt5;

int main(int argc, char **argv) {

	IOQtApplication app("isisview",true,false);
// 	app.getLogHandler(data::Debug::name(),warning)->stopBelow(notice);
	app.init(argc,argv);

	while(!app.images.empty()){
		display(app.fetchImage());
	}
    return app.exec();
}

