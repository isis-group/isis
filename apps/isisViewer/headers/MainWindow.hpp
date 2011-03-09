/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#ifndef MAINWINDOW_CPP_
#define MAINWINDOW_CPP_

#include "MainWindowBase.hpp"
#include "ViewerCoreBase.hpp"
#include "QGLWidgetImplementation.hpp"

namespace isis {
namespace viewer {

class MainWindow : public MainWindowBase
{
	

public:
	MainWindow( boost::shared_ptr<ViewerCoreBase> core  );
	

private:
	struct Slot {
		MainWindow &parent;
		Slot( MainWindow &p ) : parent( p ) {}
	};
	
	boost::shared_ptr<ViewerCoreBase> m_ViewerCore;
	
	boost::shared_ptr<QGLWidgetImplementation> axialWidget;
	


private:
	struct RefreshIntensityDisplay : Slot {
		RefreshIntensityDisplay( MainWindow &p ): Slot( p ) {}
		void operator() ( const size_t & );
	} my_RefreshIntensityDisplay;
	struct RefreshCoordsDisplay: Slot {
		RefreshCoordsDisplay( MainWindow &p ) : Slot( p ) {}
		void operator() ( const size_t &, const size_t &, const size_t &, const size_t & );
	} my_RefreshCoordsDisplay;
};


}
} //end namespaces

#endif /* MAINWINDOW_CPP_ */
