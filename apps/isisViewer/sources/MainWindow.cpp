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
	connect( ui.goButton, SIGNAL( clicked() ), this, SLOT( go() ) );

	m_AxialWidget = new QGLWidgetImplementation( core, ui.axialWidget, OrientationHandler::axial );
	m_ViewerCore->registerWidget( "axialView", m_AxialWidget );

	m_SagittalWidget = m_AxialWidget->createSharedWidget( ui.sagittalWidget, OrientationHandler::sagittal );
	m_ViewerCore->registerWidget( "sagittalView", m_SagittalWidget );

	//TODO only view 2 widgets since my computer is too dumb to manage 3 ones
	//  m_CoronalWidget = m_AxialWidget->createSharedWidget( ui.coronalWidget, OrientationHandler::coronal );
	//  m_ViewerCore->registerWidget( "coronalView", m_CoronalWidget );
	//

}


void MainWindow::go()
{
	size_t x = ui.xEdit->text().toInt();
	size_t y = ui.yEdit->text().toInt();
	size_t z = ui.zEdit->text().toInt();
	m_AxialWidget->lookAtVoxel( x, y, z );
	m_SagittalWidget->lookAtVoxel( x, y, z );

	//  m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("axialView")->lookAtVoxel(x,y,z);
	//  m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("sagittalView")->lookAtVoxel(x,y,z);
	//  m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("axialWidget")->lookAtVoxel(ui.xEdit->text().toInt());
}

}
} //end namespace
