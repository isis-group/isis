/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#ifndef MAINWINDOW_CPP_
#define MAINWINDOW_CPP_

#include "MainWindowBase.hpp"
#include "ViewerCore.hpp"
#include "QGLWidgetImplementation.hpp"

namespace isis
{
namespace viewer
{

class MainWindow : public MainWindowBase
{

	Q_OBJECT

public:
	MainWindow( ViewerCore *core  );

private:
	struct Slot {
		MainWindow &parent;
		Slot( MainWindow &p ) : parent( p ) {}
	};
	ViewerCore *m_ViewerCore;

protected Q_SLOTS:
	//TODO debug
	void go();


private:
	QGLWidgetImplementation *m_AxialWidget;
	QGLWidgetImplementation *m_CoronalWidget;
	QGLWidgetImplementation *m_SagittalWidget;
};


}
} //end namespaces

#endif /* MAINWINDOW_CPP_ */
