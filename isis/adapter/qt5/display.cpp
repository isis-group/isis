#include "display.hpp"
#include <QApplication>
#include <QFile>
#include "common.hpp"
#include "simpleimageview.hpp"

void isis::qt5::display(data::Image img, std::string title)
{
	if(qApp)
		(new SimpleImageView(img,QString::fromStdString(title)))->show(); // create image and show it
	else { // if there is no qApp
		QFile cmdline_file("/proc/self/cmdline");

		if(!cmdline_file.open(QIODevice::ReadOnly)){
			LOG(isis::qt5::Runtime,isis::error) 
				<< "Failed to open " << isis::util::MSubject("/proc/self/cmdline") << " and thus can't create widget";
			return;
		}

		QList<QByteArray> cmdline=cmdline_file.readAll().split(0);
		static int argc=cmdline.length();
		static char **argv=new char*[argc];
		LOG(isis::qt5::Debug,isis::info) << "Creating QGuiApplication dummy ... ";
		QApplication *app=new QApplication(argc,argv);
		(new SimpleImageView(img,QString::fromStdString(title)))->show(); // create image and show it
		app->exec();
	}
}

