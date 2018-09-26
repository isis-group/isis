#include "display.hpp"
#include <QApplication>
#include <QFile>
#include "common.hpp"
#include "simpleimageview.hpp"

void isis::qt5::display(data::Chunk chk, std::string title)
{
	// make sure we have the needed props
	chk.refValueAsOr("acquisitionNumber",0);
	chk.refValueAsOr("rowVec",     util::fvector3{1,0,0});
	chk.refValueAsOr("columnVec",  util::fvector3{0,1,0});
	chk.refValueAsOr("indexOrigin",util::fvector3{0,0,0});
	chk.refValueAsOr("voxelSize",  util::fvector3{1,1,1});
	display(data::Image(chk),title);
}


void isis::qt5::display(data::Image img, std::string title)
{
	if(img.getDimSize(0)>32768){
		LOG(Runtime,warning) << "Will not display " << img.getSizeAsString() << "-image as it is to wide (max: 32768)";
	} else if(img.getDimSize(1)>32768){
		LOG(Runtime,warning) << "Will not display " << img.getSizeAsString() << "-image as it is to tall (max: 32768)";
	} else if(qApp)
		(new SimpleImageView(img,QString::fromStdString(title)))->show(); // create image and show it
	else { // if there is no qApp @todo pretty dirty hack only working on linux
		QFile cmdline_file("/proc/self/cmdline");

		if(!cmdline_file.open(QIODevice::ReadOnly)){
			LOG(isis::qt5::Runtime,isis::error) 
				<< "Failed to open " << isis::util::MSubject("/proc/self/cmdline") << " and thus can't create widget";
			return;
		}

		QList<QByteArray> cmdline=cmdline_file.readAll().split(0);
		static int argc=cmdline.length();
		static char **argv=new char*[argc];
		for(int i=0;i<cmdline.length();i++){
			argv[i]=new char[256];
			strncpy(argv[i],cmdline[i],cmdline[i].length());
		}
		
		LOG(isis::qt5::Debug,isis::info) << "Creating QGuiApplication dummy ... ";
		QApplication *app=new QApplication(argc,argv);
		(new SimpleImageView(img,QString::fromStdString(title)))->show(); // create image and show it
		app->exec();
	}
}

