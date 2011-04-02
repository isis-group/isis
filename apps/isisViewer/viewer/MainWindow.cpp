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

MainWindow::MainWindow( QViewerCore *core )
	: m_ViewerCore( core )
{



	m_AxialWidget = new QGLWidgetImplementation( core, ui.axialWidget, GLOrientationHandler::axial );
	m_ViewerCore->registerWidget( "axialView", m_AxialWidget );

	m_SagittalWidget = m_AxialWidget->createSharedWidget( ui.sagittalWidget, GLOrientationHandler::sagittal );
	m_ViewerCore->registerWidget( "sagittalView", m_SagittalWidget );

	m_CoronalWidget = m_AxialWidget->createSharedWidget( ui.coronalWidget, GLOrientationHandler::coronal );
	m_ViewerCore->registerWidget( "coronalView", m_CoronalWidget );

	connect( ui.goButton, SIGNAL( clicked() ), this, SLOT( go() ) );
	connect( m_AxialWidget, SIGNAL( voxelCoordChanged( util::ivector4 ) ), this, SLOT( voxelCoordChanged( util::ivector4 ) ) );
	connect( m_SagittalWidget, SIGNAL( voxelCoordChanged( util::ivector4 ) ), this, SLOT( voxelCoordChanged( util::ivector4 ) ) );
	connect( m_CoronalWidget, SIGNAL( voxelCoordChanged( util::ivector4 ) ), this, SLOT( voxelCoordChanged( util::ivector4 ) ) );
	connect( ui.timestepSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT ( timestepChanged( int ) ) );
}


void MainWindow::voxelCoordChanged( util::ivector4 coords )
{
	//TODO where has this to be placed???
	BOOST_FOREACH( QViewerCore::WidgetMap::const_reference widget, m_ViewerCore->getWidgets() ) {
		dynamic_cast<QGLWidgetImplementation *>( widget.second )->lookAtVoxel( coords );
	}
	data::Chunk ch = m_ViewerCore->getCurrentImage().getImage().getChunk( coords[0], coords[1], coords[2], coords[3] );

	switch( ch.getTypeID() ) {
	case data::ValuePtr<int8_t>::staticID:
		displayIntensity<int8_t>( coords );
		break;
	case data::ValuePtr<uint8_t>::staticID:
		displayIntensity<uint8_t>( coords );
		break;
	case data::ValuePtr<int16_t>::staticID:
		displayIntensity<int16_t>( coords );
		break;
	case data::ValuePtr<uint16_t>::staticID:
		displayIntensity<uint16_t>( coords );
		break;
	case data::ValuePtr<int32_t>::staticID:
		displayIntensity<int32_t>( coords );
		break;
	case data::ValuePtr<uint32_t>::staticID:
		displayIntensity<uint32_t>( coords );
		break;
	case data::ValuePtr<float>::staticID:
		displayIntensity<float>( coords );
		break;
	case data::ValuePtr<double>::staticID:
		displayIntensity<double>( coords );
		break;
	}

}


void MainWindow::timestepChanged( int timestep )
{
	BOOST_FOREACH( QViewerCore::WidgetMap::const_reference widget, m_ViewerCore->getWidgets() ) {
		dynamic_cast<QGLWidgetImplementation *>( widget.second )->timestepChanged( timestep );
	}
}


void MainWindow::go()
{
	//TODO debug
	//  size_t x = ui.xEdit->text().toInt();
	//  size_t y = ui.yEdit->text().toInt();
	//  size_t z = ui.zEdit->text().toInt();
	//  m_AxialWidget->lookAtVoxel( x, y, z );
	//  m_SagittalWidget->lookAtVoxel( x, y, z );
	//  m_CoronalWidget->lookAtVoxel( x, y, z );

	//  m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("axialView")->lookAtVoxel(x,y,z);
	//  m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("sagittalView")->lookAtVoxel(x,y,z);
	//  m_ViewerCore->getWidgetAs<QGLWidgetImplementation>("axialWidget")->lookAtVoxel(ui.xEdit->text().toInt());
}



}
} //end namespace
