/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#ifndef MAINWINDOW_CPP_
#define MAINWINDOW_CPP_

#include <QtGui>
#include "ui_isisViewer.h"
#include "ViewControl.hpp"
#include "Adapter/vtkAdapter.hpp"

namespace isis {

namespace viewer {

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(  const util::slist&, QMainWindow *parent = 0 );

private:
	Ui::isisViewer ui;
	ViewControl m_Viewer;
	void setUpGui( void );
	void createAndSendImageMap( const util::slist& );
	struct Slot{
		MainWindow &parent;
		Slot( MainWindow& p ) : parent( p ) {}
	};

private Q_SLOTS:
	void timeStepChanged( int );
	void checkPhysicalChanged( bool );

Q_SIGNALS:
	void clicked( bool );
	void valueChanged( int );

//slots
private:
	struct RefreshIntensityDisplay : Slot
	{
		RefreshIntensityDisplay( MainWindow& p ):Slot(p) {}
		void operator() ( const size_t& );
	}my_RefreshIntensityDisplay;
	struct RefreshCoordsDisplay : Slot
	{
		RefreshCoordsDisplay( MainWindow &p ) : Slot(p) {}
		void operator() ( const size_t&, const size_t&, const size_t&, const size_t& );
	}my_RefreshCoordsDisplay;
};


}} //end namespaces

#endif /* MAINWINDOW_CPP_ */
