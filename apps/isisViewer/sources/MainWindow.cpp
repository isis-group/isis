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
	m_AxialWidget = new QGLWidgetImplementation( core, ui.axialWidget, OrientationHandler::axial );
	m_ViewerCore->registerWidget( "axialView", m_AxialWidget );
	
	m_SagittalWidget = m_AxialWidget->createSharedWidget( ui.sagittalWidget, OrientationHandler::sagittal );
	m_ViewerCore->registerWidget( "sagittalView", m_SagittalWidget );

	//TODO only view 2 widgets since my computer is too dumb to manage 3 ones
// 	m_CoronalWidget = m_AxialWidget->createSharedWidget( ui.coronalWidget, OrientationHandler::coronal );
// 	m_ViewerCore->registerWidget( "coronalView", m_CoronalWidget );
// 	

}

}
} //end namespace
