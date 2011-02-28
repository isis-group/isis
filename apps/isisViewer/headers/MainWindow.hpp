/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#ifndef MAINWINDOW_CPP_
#define MAINWINDOW_CPP_

#include "QGLView.h"
#include <QtGui>
#include <QtOpenGL>
#include "ui_isisViewer.h"
#include <DataStorage/image.hpp>

namespace isis
{

namespace viewer
{

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow( std::list<data::Image>, QMainWindow *parent = 0 );

private:
	QGLView *viewAxial;
	QGLView *viewSagittal;
	QGLView *viewCoronal;
	
	Ui::isisViewer ui;
	struct Slot {
		MainWindow &parent;
		Slot( MainWindow &p ) : parent( p ) {}
	};

private Q_SLOTS:

Q_SIGNALS:
	void clicked( bool );
	void valueChanged( int );

	//slots
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
