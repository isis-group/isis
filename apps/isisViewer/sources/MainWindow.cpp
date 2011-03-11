/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"
#include <common.hpp>


namespace isis
{
namespace viewer
{

MainWindow::MainWindow( ViewerCore *core )
	: m_ViewerCore( core )
{
	ui.setupUi( this );
	m_AxialWidget = new QGLWidgetImplementation( core, ui.widgetAxial, 0, QGLWidgetImplementation::axial );
	m_ViewerCore->registerWidget( "axialView", m_AxialWidget );
	m_CoronalWidget = new QGLWidgetImplementation( core, ui.widgetCoronal, m_AxialWidget, QGLWidgetImplementation::coronal );
	m_ViewerCore->registerWidget( "coronalView", m_CoronalWidget );


}

}
} //end namespace
