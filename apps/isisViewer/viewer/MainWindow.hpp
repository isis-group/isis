/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#ifndef MAINWINDOW_CPP_
#define MAINWINDOW_CPP_

#include "MainWindowBase.hpp"
#include "QViewerCore.hpp"
#include "QGLWidgetImplementation.hpp"

namespace isis
{
namespace viewer
{

class MainWindow : public MainWindowBase
{

	Q_OBJECT

public:
	MainWindow( QViewerCore *core  );

private:

	QViewerCore *m_ViewerCore;

public Q_SLOTS:
	void voxelCoordChanged( util::ivector4 );
	void exitProgram();
	void imagesChanged( DataContainer::ImageMapType );
	void checkImageStack( QListWidgetItem* item );
	void openImage();


private:
	QGLWidgetImplementation *m_AxialWidget;
	QGLWidgetImplementation *m_CoronalWidget;
	QGLWidgetImplementation *m_SagittalWidget;
	QGLWidgetImplementation *m_MasterWidget;
	
	
	
	template<typename TYPE> void displayIntensity( util::ivector4 coords ) {
		QString str;
		str.setNum( m_ViewerCore->getCurrentImage().getImage()->voxel<TYPE>( coords[0], coords[1], coords[2], coords[3] ) );
		ui.pxlIntensityContainer->display( str );
	}

};


}
} //end namespaces

#endif /* MAINWINDOW_CPP_ */
