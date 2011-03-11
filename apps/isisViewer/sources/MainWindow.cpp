/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"
#include <common.hpp>


namespace isis {
namespace viewer {

MainWindow::MainWindow( ViewerCore *core )
	  : m_ViewerCore( core )
{
	ui.setupUi( this );
	axialWidget = new QGLWidgetImplementation( core, ui.widgetAxial, QGLWidgetImplementation::axial );
 	m_ViewerCore->registerWidget("axialView", axialWidget );
	
	
}

}} //end namespace
