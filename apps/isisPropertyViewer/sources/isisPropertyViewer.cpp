/****************************************************************
 *
 * Copyright (C) <year> Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
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
#include "isisPropertyViewer.hpp"

#include <stdlib.h>

isisPropertyViewer::isisPropertyViewer( const isis::util::slist &fileList, QMainWindow *parent )
		: QMainWindow( parent )
{
// 	isis::util::DefaultMsgPrint::stopBelow( isis::warning );
	ui.setupUi( this );
	//connect itemDoubleClicked
	QObject::connect( this->ui.treeWidget, SIGNAL( itemDoubleClicked( QTreeWidgetItem*, int ) ), this, SLOT( edit_item( QTreeWidgetItem*, int ) ) );
	this->ui.treeWidget->setColumnCount( 2 );
	QStringList header;
	header << tr( "Property" ) << tr( "Value" );
	this->ui.treeWidget->setHeaderLabels( header );

	for ( isis::util::slist::const_iterator fileIterator = fileList.begin(); fileIterator != fileList.end(); fileIterator++ ) {
		addFileToTree( tr( fileIterator->c_str() ) );		
	}
}

Qt::ItemFlags isisPropertyViewer::flags( const QModelIndex &index ) const
{
	if ( not index.isValid() ) {
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
	addFileToTree( fileName );
}

void isisPropertyViewer::on_action_Clear_activated()
{
	this->ui.treeWidget->clear();
}


void isisPropertyViewer::addFileToTree( const QString &fileName )
{
	if ( not fileName.isEmpty() ) {
		this->setStatusTip( fileName );
		isis::data::ImageList inputImageList = isis::data::IOFactory::load( fileName.toStdString(), "" );

		std::cout << "Loaded " << inputImageList.size() << " Images" << std::endl;
		BOOST_FOREACH(isis::data::ImageList::const_reference ref, inputImageList) {
			createTree( ref, fileName );
			m_propHolder.addPropMapFromImage( ref, fileName );
		}

		if ( inputImageList.empty() ) {
			LOG( isis::DataLog, isis::error ) << "Input image list is empty!";
			QMessageBox::information( this, "Error", "Input image list is empty!" );
		}
	}
	else std::cout << "is leer" << std::endl;
		
}

void isisPropertyViewer::createTree( const boost::shared_ptr<isis::data::Image> image, const QString& fileName )
{
	QString headerProp, headerVal;
	QStringList header;
	headerProp.sprintf( "Image" );
	headerVal = fileName;
	header << headerProp << headerVal;
	QTreeWidgetItem* headItem = new QTreeWidgetItem( header );
	this->ui.treeWidget->addTopLevelItem( headItem );
	m_keyList = image->keys();

	for ( PropKeyListType::const_iterator propIterator = m_keyList.begin(); propIterator != m_keyList.end(); propIterator++ ) {
		addPropToTree( image, propIterator, headItem );
	}

	unsigned short chunkCounter = 0;

	//go through all the chunks
	for ( isis::data::Image::ChunkIterator chunkIterator = image->chunksBegin(); chunkIterator != image->chunksEnd(); chunkIterator++ ) {
		m_keyList = chunkIterator->keys();

		if ( not m_keyList.empty() ) {
			QString headerProp, headerVal;
			QStringList header;
			headerProp.sprintf( "Chunk" );
			headerVal.sprintf( "%d", chunkCounter );
			header << headerProp << headerVal;
			QTreeWidgetItem* chunkItem = new QTreeWidgetItem( header );
			headItem->addChild( chunkItem );

			for ( PropKeyListType::const_iterator propIterator = m_keyList.begin(); propIterator != m_keyList.end(); propIterator++ ) {
				addPropToTree( image, propIterator, chunkItem );
			}
		}

		chunkCounter++;
	}

	QString chunkString;
	chunkString.sprintf( "%d", chunkCounter );
	addChildToItem( headItem, tr( "chunks" ), chunkString );
}

void isisPropertyViewer::addChildToItem( QTreeWidgetItem* item, const QString& prop, const QString& val ) const
{
	QStringList stringList;
	stringList << prop << val;
	QTreeWidgetItem* newItem = new QTreeWidgetItem( stringList );
	item->addChild( newItem );
}

void isisPropertyViewer::addPropToTree( const boost::shared_ptr<isis::data::Image> image, const PropKeyListType::const_iterator &propIterator, QTreeWidgetItem* currentHeadItem )
{
	LOG_IF( image->getPropertyValue( *propIterator ).empty(), isis::CoreLog, isis::error ) << "Property " << *propIterator << " is empty";

	if ( not image->getPropertyValue( *propIterator ).empty() ) {
		if ( image->getPropertyValue( *propIterator )->is<isis::util::fvector4>() ) {
			std::vector<QString> stringVec;
			QTreeWidgetItem* vectorItem = new QTreeWidgetItem( QStringList( tr( propIterator->c_str() ) ) );
			currentHeadItem->addChild( vectorItem );

			for ( unsigned short dim = 0; dim < 4; dim++ ) {
				QString tmp = "";
				tmp.sprintf( "%f", image->getProperty<isis::util::fvector4>( *propIterator )[dim] );
				stringVec.push_back( tmp );
			}

			addChildToItem( vectorItem, tr( "x" ), stringVec[0] );
			addChildToItem( vectorItem, tr( "y" ), stringVec[1] );
			addChildToItem( vectorItem, tr( "z" ), stringVec[2] );
			addChildToItem( vectorItem, tr( "t" ), stringVec[3] );
		} else {
			addChildToItem( currentHeadItem, tr( propIterator->c_str() ), tr( image->getPropertyValue( *propIterator ).toString( true ).c_str() ) );
		}
	} else {
		addChildToItem( currentHeadItem, tr( propIterator->c_str() ), tr( "empty" ) );
	}
};

void isisPropertyViewer::edit_item( QTreeWidgetItem* item, int val )
{
	if( val == 1)
	{
		QMessageBox::information( this, "isisPropertyViewer", "Edit mode not yet implemented!" );
	}
}


