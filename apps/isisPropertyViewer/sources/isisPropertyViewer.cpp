
#include <QtGui>
#include "isisPropertyViewer.h"

#include <stdlib.h>

isisPropertyViewer::isisPropertyViewer( QMainWindow *parent )
		: QMainWindow( parent )
{
	ui.setupUi( this );
	//connect itemDoubleClicked
	QObject::connect(this->ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(edit_item(QTreeWidgetItem*, int)));
	this->ui.treeWidget->setColumnCount( 2 );
	QStringList header;
	header << tr( "Property" ) << tr( "Value" );
	this->ui.treeWidget->setHeaderLabels( header );
}

Qt::ItemFlags isisPropertyViewer::flags( const QModelIndex &index) const
{
	if( not index.isValid())
	{
		return 0;
	}
	return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
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
			isis::data::ImageList::const_iterator imageIterator;
			for ( imageIterator = inputImageList.begin(); imageIterator != inputImageList.end(); imageIterator++ ) {
				addImageToTree( *imageIterator, fileName );
			}
		}
		if ( inputImageList.empty() ) {
			QMessageBox::information(this, "Error", "Input image list is empty!");
		}
	}
}

void isisPropertyViewer::on_action_Clear_activated()
{
	this->ui.treeWidget->clear();
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
		LOG_IF(image->getPropertyValue(*propIterator).empty(),isis::CoreLog,isis::error) << "Property " << *propIterator << " is empty";
		if( not image->getPropertyValue(*propIterator).empty())
		{
			if(image->getPropertyValue(*propIterator)->is<isis::util::fvector4>())
			{
				std::vector<QString> stringVec;
				QTreeWidgetItem* vectorItem = new QTreeWidgetItem(QStringList(tr(propIterator->c_str())));
				headItem->addChild(vectorItem);
				for(unsigned short dim=0; dim < 4; dim++)
				{
					QString tmp= "";
					tmp.sprintf("%f",image->getProperty<isis::util::fvector4>(*propIterator)[dim]);
					stringVec.push_back(tmp);
				}
				addChildToItem(vectorItem, tr("x"), stringVec[0]);
				addChildToItem(vectorItem, tr("y"), stringVec[1]);
				addChildToItem(vectorItem, tr("z"), stringVec[2]);
				addChildToItem(vectorItem, tr("t"), stringVec[3]);
				
			} else
			{
				addChildToItem(headItem, tr(propIterator->c_str()), tr(image->getPropertyValue(*propIterator).toString(true).c_str()));
			}
		} else
		{
			addChildToItem(headItem, tr(propIterator->c_str()), tr("empty"));
		}
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
			headerVal.sprintf("%d", chunkCounter);
			header << headerProp << headerVal;
			QTreeWidgetItem* chunkItem = new QTreeWidgetItem(header);
			headItem->addChild(chunkItem);
			for(PropKeyListType::const_iterator propIterator = m_keyList.begin(); propIterator != m_keyList.end(); propIterator++)
			{
				addChildToItem(chunkItem, tr(propIterator->c_str()), tr(chunkIterator->getPropertyValue(*propIterator).toString(true).c_str()));
			}
		}
		chunkCounter++;
	}
	QString chunkString;
	chunkString.sprintf("%d", chunkCounter);
	addChildToItem(headItem, tr("chunks"), chunkString); 
}

void isisPropertyViewer::addChildToItem( QTreeWidgetItem* item, const QString& prop, const QString& val) const
{
	QStringList stringList;
	stringList << prop << val;
	QTreeWidgetItem* newItem = new QTreeWidgetItem(stringList);
	item->addChild(newItem);	
}


void isisPropertyViewer::edit_item(QTreeWidgetItem* item, int val)
{	
	QMessageBox::information(this, "isisPropertyViewer", "Edit mode not yet implemented!");
}


