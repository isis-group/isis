#include <iostream>
#include <isis/adapter/qt5/common.hpp>
#include <isis/adapter/qt5/qtapplication.hpp>
#include <isis/adapter/qt5/simpleimageview.hpp>

int main(int argc, char **argv) {

	isis::qt5::IOQtApplication app("isisview",true,false);
// 	app.getLogHandler(isis::data::Debug::name(),isis::warning)->stopBelow(isis::notice);
	app.init(argc,argv);

	while(!app.images.empty()){
		isis::qt5::display(app.fetchImage());
	}
    return app.exec();
}

