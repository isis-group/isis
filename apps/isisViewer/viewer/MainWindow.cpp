/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"
#include <common.hpp>
#include <DataStorage/io_factory.hpp>

namespace isis
{
namespace viewer
{

MainWindow::MainWindow( QViewerCore *core )
	: m_ViewerCore( core )
{
	
	connect(ui.action_Exit, SIGNAL(triggered()), this, SLOT( exitProgram() ) );
	connect(ui.actionShow_labels, SIGNAL(toggled(bool)), m_ViewerCore, SLOT(setShowLabels(bool)));
	connect(core, SIGNAL( emitImagesChanged(DataContainer::ImageMapType)), this, SLOT( imagesChanged(DataContainer::ImageMapType) ) );
	connect(ui.imageStack, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT( checkImageStack(QListWidgetItem*)));
	connect(ui.action_Open_Image, SIGNAL(triggered()), this, SLOT(openImage()));
	
	//we need a master widget to keep opengl running in case all visible widgets were closed
	
	m_MasterWidget = new QGLWidgetImplementation( core, 0, axial );
	
	m_AxialWidget =  m_MasterWidget->createSharedWidget( ui.axialWidget, axial );
	m_ViewerCore->registerWidget( "axialView", m_AxialWidget );

	m_CoronalWidget = m_MasterWidget->createSharedWidget( ui.coronalWidget, coronal );
	m_ViewerCore->registerWidget( "coronalView", m_CoronalWidget );

	m_SagittalWidget = m_MasterWidget->createSharedWidget( ui.sagittalWidget, sagittal );
	m_ViewerCore->registerWidget( "sagittalView", m_SagittalWidget );

	m_ViewerCore->registerWidget( "timestepSpinBox", ui.timestepSpinBox, QViewerCore::timestep_changed );
	ui.actionShow_labels->setCheckable(true);
	ui.actionShow_labels->setChecked(false);
	
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

void MainWindow::imagesChanged(DataContainer::ImageMapType imageMap )
{	
	ui.imageStack->clear();
	BOOST_FOREACH( DataContainer::ImageMapType::const_reference imageRef, imageMap )
	{
		QListWidgetItem *item = new QListWidgetItem;
		QString sD = imageRef.second.getPropMap().getPropertyAs<std::string>("sequenceDescription").c_str();
		if (!sD.toStdString().empty()) {
			item->setText(QString( imageRef.second.getFileNames().front().c_str() ) );	
		} else {
			item->setText(QString( imageRef.second.getFileNames().front().c_str() ) );
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

void MainWindow::openImage()
{
	QStringList filenames = QFileDialog::getOpenFileNames(this, 
														  tr("Open images"), 
														  m_CurrentPath,
														  tr("Image Files (*.v *.nii *.hdr *.dcm *.ima"));
	QDir dir;
	m_CurrentPath = dir.absoluteFilePath(filenames.front());
	if(!filenames.empty()) {
		std::list<data::Image> imgList;
		util::slist pathList;
		BOOST_FOREACH( QStringList::const_reference filename, filenames)
		{
			std::list<data::Image> tempImgList = data::IOFactory::load( filename.toStdString() ,"", "");
			pathList.push_back( filename.toStdString() );
			BOOST_FOREACH(std::list<data::Image>::const_reference image, tempImgList)
			{
				imgList.push_back(image);
			}
			
		}
		m_ViewerCore->addImageList(imgList, ImageHolder::anatomical_image, pathList );
		m_ViewerCore->updateScene();
	}
}


void MainWindow::checkImageStack(QListWidgetItem* item)
{
	if(item->checkState() == Qt::Checked)
	{
		m_ViewerCore->getDataContainer().getImageHolder( item->text().toStdString() ).setVisible(true);
		
	} else if( item->checkState() == Qt::Unchecked ) {
		m_ViewerCore->getDataContainer().getImageHolder( item->text().toStdString() ).setVisible(false);
	}
	
	m_ViewerCore->updateScene();
	
}




void MainWindow::exitProgram()
{
	close();
}


}
} //end namespace
