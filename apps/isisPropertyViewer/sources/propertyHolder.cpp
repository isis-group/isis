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

#include "propertyHolder.hpp"
#include "DataStorage/io_factory.hpp"

bool PropertyHolder::addPropMapFromImage( const boost::shared_ptr<isis::data::Image> image, const QString &filename )
{
	isis::util::PropMap tmpPropMap = *image;
	m_propHolderMap[filename.toStdString()] = tmpPropMap;
	m_propChanged[filename.toStdString()] = false;
	return true;
}

void PropertyHolder::saveIt( const QString &fileNameAs, const bool SaveAs )
{
	for ( std::map<std::string, bool>::iterator boolIter = m_propChanged.begin(); boolIter != m_propChanged.end(); boolIter++ ) {
		if ( boolIter->second ) {
			isis::data::ImageList imageList = isis::data::IOFactory::load( boolIter->first , "" );
			isis::data::ImageList tmpImageList;
			BOOST_FOREACH( isis::data::ImageList::reference image, imageList ) {
				const isis::util::PropMap &tmpMap( m_propHolderMap.find( boolIter->first )->second ) ;
				static_cast<isis::util::PropMap &>( *image ) = tmpMap;
				tmpImageList.push_back( image );
			}

			if ( not SaveAs ) {
				std::string fileName = boolIter->first;
				size_t pos = fileName.find( "." );
				fileName.insert( pos, std::string( ".chg" ) );
				isis::data::IOFactory::write( tmpImageList, fileName, "" );
			} else {
				isis::data::IOFactory::write( tmpImageList, fileNameAs.toStdString(), "" );
			}

			m_propChanged.find( boolIter->first )->second = false;
		}
	}
}