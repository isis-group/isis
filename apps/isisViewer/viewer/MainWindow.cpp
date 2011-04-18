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
	connect(ui.action_Exit, SIGNAL(triggered()), this, SLOT( exitProgram() ) );
	connect(core, SIGNAL( emitImagesChanged(DataContainer::ImageFileMapType)), this, SLOT( imagesChanged(DataContainer::ImageFileMapType) ) );
	connect(ui.imageStack, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT( checkImageStack(QListWidgetItem*)));
	
	m_AxialWidget = new QGLWidgetImplementation( core, ui.axialWidget, GLOrientationHandler::axial );
	m_ViewerCore->registerWidget( "axialView", m_AxialWidget );

	m_CoronalWidget = m_AxialWidget->createSharedWidget( ui.coronalWidget, GLOrientationHandler::coronal );
	m_ViewerCore->registerWidget( "coronalView", m_CoronalWidget );

	m_SagittalWidget = m_AxialWidget->createSharedWidget( ui.sagittalWidget, GLOrientationHandler::sagittal );
	m_ViewerCore->registerWidget( "sagittalView", m_SagittalWidget );


	m_ViewerCore->registerWidget( "timestepSpinBox", ui.timestepSpinBox, QViewerCore::timestep_changed );
	m_ViewerCore->registerWidget( "showLabels", ui.checkLabels, QViewerCore::show_labels );
	
}


void MainWindow::voxelCoordChanged( util::ivector4 coords )
{
	//TODO where has this to be placed???
	BOOST_FOREACH( QViewerCore::WidgetMap::const_reference widget, m_ViewerCore->getWidgets() ) {
		dynamic_cast<QGLWidgetImplementation *>( widget.second )->lookAtVoxel( coords );
	}
	data::Chunk ch = m_ViewerCore->getCurrentImage().getImage()->getChunk( coords[0], coords[1], coords[2], coords[3] );

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

void MainWindow::imagesChanged(DataContainer::ImageFileMapType imageMap )
{	
	ui.imageStack->clear();
	BOOST_FOREACH( DataContainer::ImageFileMapType::const_reference imageRef, imageMap )
	{
		QListWidgetItem *item = new QListWidgetItem;
		QString sD = imageRef.second.getPropMap().getPropertyAs<std::string>("sequenceDescription").c_str();
		if (!sD.toStdString().empty()) {
			item->setText(QString( imageRef.first.first.c_str() ) );	
		} else {
			item->setText(QString( imageRef.first.first.c_str() ) );
		}
		item->setFlags( Qt::ItemIsEnabled | Qt::ItemIsUserCheckable );
		
		if( imageRef.second.getImageState().visible ) {
			item->setCheckState(Qt::Checked);
		} else {
			item->setCheckState(Qt::Unchecked);
		}
		ui.imageStack->addItem(item);
		
	}
}

void MainWindow::checkImageStack(QListWidgetItem* item)
{
	if(item->checkState() == Qt::Checked)
	{
		std::cout << item->text().toStdString() << " is active!" << std::endl << std::flush;
	} else {
		std::cout << item->text().toStdString() << " is not active!" << std::endl << std::flush;
	}
	
}




void MainWindow::exitProgram()
{
	close();
}


}
} //end namespace
