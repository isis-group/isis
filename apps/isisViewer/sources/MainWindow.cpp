/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"
#include <common.hpp>
#include "QGLWidgetImplementation.hpp"

namespace isis {
namespace viewer {

MainWindow::MainWindow( ViewerCore *core )
	  : m_ViewerCore( core )
{
	ui.setupUi( this );
	QGLWidgetImplementation* axialWidget = new QGLWidgetImplementation( core, ui.widgetAxial, QGLWidgetImplementation::axial );
	m_ViewerCore->registerWidget("axialView", boost::shared_ptr<QGLWidgetImplementation> (axialWidget) );
// 	m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("axialView")->paint();
	
	
}

}} //end namespace
