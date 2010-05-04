
#include <QtGui>
#include "isisPropertyViewer.h"


isisPropertyViewer::isisPropertyViewer( QMainWindow *parent )
		: QMainWindow( parent )
{
	ui.setupUi( this );
	this->ui.treeWidget->setColumnCount( 2 );
	QStringList header;
	header << tr( "Property" ) << tr( "Value" );
	this->ui.treeWidget->setHeaderLabels( header );
}

void isisPropertyViewer::on_action_Close_activated()
{
	this->close();
}

void isisPropertyViewer::on_action_Open_activated()
{
	QString fileName = QFileDialog::getOpenFileName( this, tr( "Open Image" ), QDir::currentPath(), "*.nii *.v" );

	if ( not fileName.isEmpty() ) {
		this->setStatusTip( "Opening " + fileName + " ..." );
		isis::data::ImageList inputImageList = isis::data::IOFactory::load( fileName.toStdString(), "" );
		LOG_IF( inputImageList.empty(), isis::DataLog, isis::error ) << "Input image list is empty!";

		if ( not inputImageList.empty() ) {
			this->ui.treeWidget->clear();
			isis::data::ImageList::const_iterator imageIterator;

			for ( imageIterator = inputImageList.begin(); imageIterator != inputImageList.end(); imageIterator++ ) {
				addToTree( *imageIterator );
			}
		}
	}
}

void isisPropertyViewer::addToTree( const boost::shared_ptr<isis::data::Image> image ) const
{
}

