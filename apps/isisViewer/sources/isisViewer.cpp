/****************************************************************
 *
 *  Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 *****************************************************************/

#include <QtGui>
#include "isisViewer.hpp"
#include "Adapter/vtkAdapter.hpp"


isisViewer::isisViewer( const isis::util::slist& fileList, QMainWindow *parent )
		: QMainWindow( parent )
{

	m_CurrentImagePtr = vtkImageData::New();

	m_RendererAxial = vtkRenderer::New();
	m_RendererSagittal = vtkRenderer::New();
	m_RendererCoronal = vtkRenderer::New();

	m_WindowAxial = vtkRenderWindow::New();
	m_WindowSagittal = vtkRenderWindow::New();
	m_WindowCoronal = vtkRenderWindow::New();

	m_InteractorAxial = vtkRenderWindowInteractor::New();
	m_InteractorSagittal = vtkRenderWindowInteractor::New();
	m_InteractorCoronal = vtkRenderWindowInteractor::New();

	m_InteractionStyleAxial = new ViewerInteractor(this, m_RendererAxial);
	m_InteractionStyleSagittal = new ViewerInteractor(this, m_RendererSagittal);
	m_InteractionStyleCoronal = new ViewerInteractor(this, m_RendererCoronal);

	setUpPipe();

	ui.setupUi( this );
	//connections qt

	QObject::connect( this->ui.verticalSlider, SIGNAL( valueChanged( int ) ), this, SLOT( valueChangedSagittal( int ) ) );
	QObject::connect( this->ui.verticalSlider_2, SIGNAL( valueChanged( int ) ), this, SLOT( valueChangedCoronal( int) ) );
	QObject::connect( this->ui.verticalSlider_3, SIGNAL( valueChanged( int ) ), this, SLOT( valueChangedAxial( int ) ) );

	//go through all files
	BOOST_FOREACH( isis::util::slist::const_reference refFile, fileList )
	{
		//go through all the images in one file
		isis::data::ImageList imgList = isis::data::IOFactory::load( refFile, "");
		BOOST_FOREACH( isis::data::ImageList::const_reference refImage, imgList )
		{
			boost::shared_ptr< ImageHolder > tmpVec( new ImageHolder );
			tmpVec->setImages(  refImage, isis::adapter::vtkAdapter::makeVtkImageObject( refImage  ) );
			tmpVec->setReadVec( refImage->getProperty<isis::util::fvector4>("readVec") );
			tmpVec->setPhaseVec( refImage->getProperty<isis::util::fvector4>("phaseVec") );
			tmpVec->setSliceVec( refImage->getProperty<isis::util::fvector4>("sliceVec") );
			m_ImageVector.push_back( tmpVec );
		}
	}
	if (!m_ImageVector.empty() ) {
		m_CurrentImagePtr = m_ImageVector.front()->getVTKImageData();
		m_CurrentImageHolder = m_ImageVector.front();
	}

	ui.verticalSlider->setRange(0, m_CurrentImagePtr->GetDimensions()[2]-1);
	ui.verticalSlider_2->setRange(0, m_CurrentImagePtr->GetDimensions()[1]-1);
	ui.verticalSlider_3->setRange(0, m_CurrentImagePtr->GetDimensions()[0]-1);

	BOOST_FOREACH( std::vector< boost::shared_ptr< ImageHolder > >::const_reference ref, m_ImageVector )
	{
		m_RendererAxial->AddActor( ref->getActorAxial() );
		m_RendererCoronal->AddActor( ref->getActorCoronal() );
		m_RendererSagittal->AddActor( ref->getActorSagittal() );
	}
	m_InteractorAxial->SetDesiredUpdateRate(0.1);
	m_InteractorAxial->Initialize();
	m_InteractorSagittal->Initialize();
	m_InteractorCoronal->Initialize();

	ui.qvtkWidgetCoronal->SetRenderWindow( m_WindowCoronal );
	ui.qvtkWidgetAxial->SetRenderWindow( m_WindowAxial );
	ui.qvtkWidgetSagittal->SetRenderWindow( m_WindowSagittal );
	resetCam();
}

void isisViewer::setUpPipe()
{
	m_InteractorCoronal->SetInteractorStyle( m_InteractionStyleCoronal );
	m_InteractorSagittal->SetInteractorStyle( m_InteractionStyleSagittal );
	m_InteractorAxial->SetInteractorStyle( m_InteractionStyleAxial );

	m_WindowCoronal->SetInteractor( m_InteractorCoronal );
	m_WindowSagittal->SetInteractor( m_InteractorSagittal );
	m_WindowAxial->SetInteractor( m_InteractorAxial );

	m_WindowCoronal->AddRenderer( m_RendererCoronal );
	m_WindowSagittal->AddRenderer( m_RendererSagittal );
	m_WindowAxial->AddRenderer( m_RendererAxial );

}

void isisViewer::resetCam()
{
	BOOST_FOREACH( std::vector< boost::shared_ptr< ImageHolder > >::const_reference refImg, m_ImageVector)
	{
		refImg->resetSliceCoordinates();
	}
	m_RendererAxial->ResetCamera();
	m_RendererSagittal->ResetCamera();
	m_RendererCoronal->ResetCamera();

	m_WindowAxial->Render();
	m_WindowCoronal->Render();
	m_WindowSagittal->Render();

}

//gui interactions


void isisViewer::displayIntensity( const int& x, const int& y, const int& z )
{
	isis::util::fvector4 tmpVec = isis::util::fvector4(x,y,z,0);
//	isis::util::fvector4 mappedVec = isis::viewer::mapCoordinates<float>(m_CurrentImageHolder->getOriginalMatrix(), tmpVec, m_CurrentImageHolder->getISISImage()->sizeToVector());
	QString atString;
	atString.sprintf("at %d %d %d", x, y, z);
	ui.atLabel->setText(atString);

	switch (m_CurrentImageHolder->getISISImage()->getChunk(x,y,z ).typeID() )
	{
	case isis::data::TypePtr<int8_t>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<int8_t>(x, y,z));
		break;
	case isis::data::TypePtr<u_int8_t>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<u_int8_t>(x, y,z));
		break;
	case isis::data::TypePtr<int16_t>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<int16_t>(x, y,z));
		break;
	case isis::data::TypePtr<u_int16_t>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<u_int16_t>(x, y,z));
		break;
	case isis::data::TypePtr<int32_t>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<int32_t>(x, y,z));
		break;
//	case isis::data::TypePtr<u_int32_t>::staticID:
//		ui.pxlIntensityContainer->display(m_ImageVector.front()->getISISImage()->voxel<u_int32_t>(x, y,z));
//		break;
	case isis::data::TypePtr<float>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<float>(x, y,z));
		break;
	case isis::data::TypePtr<double>::staticID:
		ui.pxlIntensityContainer->display(m_CurrentImageHolder->getISISImage()->voxel<double>(x, y,z));
		break;
	default:
		ui.pxlIntensityContainer->display(tr("Error"));
	}

}

void isisViewer::sliceChanged( const int& x, const int& y, const int& z)
{
	BOOST_FOREACH( std::vector< boost::shared_ptr< ImageHolder > >::const_reference refImg, m_ImageVector)
	{
		if ( not refImg->setSliceCoordinates(x,y,z) ) std::cout << "error during setting slicesetting!" << std::endl;
	}
	m_RendererAxial->ResetCamera();
	m_RendererSagittal->ResetCamera();
	m_RendererCoronal->ResetCamera();
	m_WindowAxial->Render();
	m_WindowCoronal->Render();
	m_WindowSagittal->Render();


}

void isisViewer::valueChangedSagittal( int val )
{
	sliceChanged( ui.verticalSlider->value(), ui.verticalSlider_2->value(), ui.verticalSlider_3->value() );

}

void isisViewer::valueChangedCoronal( int val )
{
	sliceChanged( ui.verticalSlider->value(), ui.verticalSlider_2->value(), ui.verticalSlider_3->value() );
}

void isisViewer::valueChangedAxial( int val )
{
	sliceChanged( ui.verticalSlider->value(), ui.verticalSlider_2->value(), ui.verticalSlider_3->value() );
}
