/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"

namespace isis
{

namespace viewer
{

MainWindow::MainWindow( std::list<data::Image> imgList, QMainWindow *parent )
	: QMainWindow( parent ),
	  my_RefreshIntensityDisplay( *this ),
	  my_RefreshCoordsDisplay( *this ),
	  viewAxial(new QGLView( imgList) )
{
	ui.setupUi( this );
	ui.gridLayout->addWidget( viewAxial );

}

}
}
