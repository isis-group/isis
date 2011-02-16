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

MainWindow::MainWindow( isis::data::ImageList imgList, QMainWindow *parent )
	: QMainWindow( parent ),
	  my_RefreshIntensityDisplay( *this ),
	  my_RefreshCoordsDisplay( *this ),
	  viewAxial(new QGLView ),
	  viewSagittal(new QGLView ),
	  viewCoronal(new QGLView )
{
	ui.setupUi( this );
	ui.gridLayout->addWidget( viewAxial );
	viewAxial->setImage( imgList );
	//connections qt
	QObject::connect( this->ui.checkPhysical, SIGNAL( clicked( bool ) ), this, SLOT( checkPhysicalChanged( bool ) ) );
	QObject::connect( this->ui.timeStepSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( timeStepChanged( int ) ) );
}

}
}
