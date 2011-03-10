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
	  : m_ViewerCore( core ),
	  axialWidget( boost::shared_ptr<QGLWidgetAdapter>( new QGLWidgetAdapter( m_ViewerCore ) ) )
{
	ui.setupUi( this );
	axialWidget->setParent( ui.widgetAxial );
	m_ViewerCore->registerWidget( "DeineMuddi", axialWidget );
	
	m_ViewerCore->getWidgetAs<QGLWidgetAdapter>("DeineMuddi")->paint();
	
	
}

}} //end namespace
