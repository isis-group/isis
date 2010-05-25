/****************************************************************
 *
 * Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
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
#ifndef PROPERTYHOLDER_HPP
#define PROPERTYHOLDER_HPP

#include <map>
#include <boost/shared_ptr.hpp>
#include "CoreUtils/propmap.hpp"
#include "DataStorage/image.hpp"
#include "ui_isisPropertyViewer.h"

class PropertyHolder
{
public:
	bool addPropMapFromImage( const boost::shared_ptr< isis::data::Image>, const QString& );
	std::map<std::string, isis::util::PropMap> m_propHolderMap;
	std::map<std::string, bool> m_propChanged;
	void saveIt( const QString&, const bool SaveAs );



};


#endif