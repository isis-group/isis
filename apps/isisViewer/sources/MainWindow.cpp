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

MainWindow::MainWindow( boost::shared_ptr<ViewerCoreBase> core )
	: my_RefreshIntensityDisplay( *this ),
	  my_RefreshCoordsDisplay( *this ),
	  m_ViewerCore( core ),
	  axialWidget( boost::shared_ptr<QGLWidgetImplementation>( new QGLWidgetImplementation( m_ViewerCore ) ) )
{
	ui.setupUi( this );
	axialWidget->setParent( ui.widgetAxial );
	m_ViewerCore->registerWidget( "DeineMuddi", axialWidget );
	
	m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("DeineMuddi")->paint();
	
	
}

}} //end namespace
