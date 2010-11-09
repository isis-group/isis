/*
 * MainWindow.cpp
 *
 *  Created on: Nov 3, 2010
 *      Author: tuerke
 */

#include "MainWindow.hpp"

namespace isis {

namespace viewer {

MainWindow::MainWindow( const util::slist& inFileList, QMainWindow *parent )
	: QMainWindow( parent ),
	  my_RefreshIntensityDisplay(*this),
	  my_RefreshCoordsDisplay(*this)
{
	ui.setupUi( this );
	m_Viewer.init(	ui.qvtkWidgetAxial,
				ui.qvtkWidgetSagittal,
				ui.qvtkWidgetCoronal );

	createAndSendImageMap( inFileList );

	//connections qt
	QObject::connect( this->ui.checkPhysical, SIGNAL( clicked( bool ) ), this, SLOT( checkPhysicalChanged( bool ) ) );
	QObject::connect( this->ui.timeStepSpinBox, SIGNAL( valueChanged( int ) ), this, SLOT( timeStepChanged( int ) ) );

	//connections to controller
	m_Viewer.signalList.intensityChanged.connect( my_RefreshIntensityDisplay );
	m_Viewer.signalList.mousePosChanged.connect( my_RefreshCoordsDisplay );
}


void MainWindow::createAndSendImageMap( const util::slist& fileList )
{
	ViewControl::ImageMapType imageMap;
	BOOST_FOREACH( util::slist::const_reference refFile, fileList )
	{
		//go through all the images in one file
		data::ImageList imgList = isis::data::IOFactory::load( refFile, "");
		BOOST_FOREACH( isis::data::ImageList::const_reference refImage, imgList )
		{
			adapter::vtkAdapter::ScalingType scaling;
			std::vector<vtkSmartPointer<vtkImageData> > vtkImageVector = isis::adapter::vtkAdapter::makeVtkImageObject(refImage, scaling);

			refImage->setProperty< util::fvector4 >( "imageSize", refImage->sizeToVector() );
			static_cast<util::TypeReference&>(refImage->propertyValue("scale"))=scaling.first;
			static_cast<util::TypeReference&>(refImage->propertyValue("offset"))=scaling.second;

			imageMap.push_back(std::make_pair(static_cast<util::PropMap>(*refImage), vtkImageVector ) );
		}
	}
	if(!imageMap.empty()) {
		m_Viewer.addImages(imageMap);
		setUpGui();
	}
}


void MainWindow::timeStepChanged( int timestep )
{
	m_Viewer.changeCurrentTimeStep( timestep );
}

void MainWindow::checkPhysicalChanged( bool physical )
{
	m_Viewer.checkPhysicalChanged( physical );
}

void MainWindow::setUpGui( void ) {
	if(m_Viewer.getCurrentImageHolder()->getNumberOfTimesteps() > 1 ) {
		this->ui.timeStepSpinBox->setMaximum( m_Viewer.getCurrentImageHolder()->getNumberOfTimesteps() - 1);
	} else {
		this->ui.timeStepSpinBox->setEnabled( false );
	}
}

void MainWindow::RefreshIntensityDisplay::operator()( const size_t& intensity )
{
	parent.ui.pxlIntensityContainer->display( static_cast<int>(intensity) );
}

void MainWindow::RefreshCoordsDisplay::operator()( const size_t& x, const size_t& y, const size_t& z, const size_t& t )
{
	QString atString;
	atString.sprintf("at %d %d %d", x, y, z);
	parent.ui.atLabel->setText(atString);
}



}
}
