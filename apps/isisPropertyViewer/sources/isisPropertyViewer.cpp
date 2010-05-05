
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
		this->setStatusTip( fileName );
		isis::data::ImageList inputImageList = isis::data::IOFactory::load( fileName.toStdString(), "" );
		LOG_IF( inputImageList.empty(), isis::DataLog, isis::error ) << "Input image list is empty!";

		if ( not inputImageList.empty() ) {
			this->ui.treeWidget->clear();
			isis::data::ImageList::const_iterator imageIterator;
			for ( imageIterator = inputImageList.begin(); imageIterator != inputImageList.end(); imageIterator++ ) {
				addImageToTree( *imageIterator, fileName );
			}
		}
		if ( inputImageList.empty() ) {
			this->setStatusTip("Image is empty!");
		}
	}
}

void isisPropertyViewer::addImageToTree( const boost::shared_ptr<isis::data::Image> image, const QString& fileName )
{
	QString headerProp, headerVal;
	QStringList header;
	headerProp.sprintf("Image");
	headerVal = fileName;
	header << headerProp << headerVal;
	QTreeWidgetItem* headItem = new QTreeWidgetItem(header);
	this->ui.treeWidget->addTopLevelItem(headItem);
	m_keyList = image->keys();
	for(PropKeyListType::const_iterator propIterator = m_keyList.begin(); propIterator != m_keyList.end(); propIterator++)
	{
		addChildToItem(headItem, tr(propIterator->c_str()), tr(image->getPropertyValue(*propIterator).toString(true).c_str()));
	}
	unsigned short chunkCounter=0;
	//go through all the chunks
	for(isis::data::Image::ChunkIterator chunkIterator = image->chunksBegin(); chunkIterator != image->chunksEnd(); chunkIterator++)
	{ 
		m_keyList = chunkIterator->keys();
		if( not m_keyList.empty() )
		{
			QString headerProp, headerVal;
			QStringList header;
			headerProp.sprintf("Chunk");
			headerVal.sprintf("%d", ++chunkCounter);
			header << headerProp << headerVal;
			QTreeWidgetItem* chunkItem = new QTreeWidgetItem(header);
			headItem->addChild(chunkItem);
			for(PropKeyListType::const_iterator propIterator = m_keyList.begin(); propIterator != m_keyList.end(); propIterator++)
			{
				addChildToItem(chunkItem, tr(propIterator->c_str()), tr(chunkIterator->getPropertyValue(*propIterator).toString(true).c_str()));
			}
		}
		
	}
}

void isisPropertyViewer::addChildToItem( QTreeWidgetItem* item, const QString& prop, const QString& val) const
{
	QStringList stringList;
	stringList << prop << val;
	QTreeWidgetItem* newItem = new QTreeWidgetItem(stringList);
	item->addChild(newItem);
	
}



