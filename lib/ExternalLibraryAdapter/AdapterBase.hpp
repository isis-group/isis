/****************************************************************
 *
 * <Copyright information>
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
 * Author: Erik T�rke, tuerke@cbs.mpg.de, 2009
 *
 * vtkAdapter.cpp
 *
 * Description:
 *
 *  Created on: Mar,30 2009
 *      Author: tuerke	
 ******************************************************************/
#ifndef ADAPTERBASE_HPP_
#define ADAPTERBASE_HPP_

//internal includes 
//local includes
#include "DataStorage/image.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/common.hpp"

//external includes 
#include <boost/smart_ptr.hpp>


namespace isis {namespace adapter{
	
class AdapterBase {
public:
	enum ChunkArrangement 
	{
		NoArrangement,
		AlongX,
		AlongY,
		AlongZ
	};
protected:
	static bool checkChunkDataType(const boost::shared_ptr<data::Image>);
};
	

//implementations	
bool AdapterBase::checkChunkDataType(const boost::shared_ptr<data::Image> image)
{
	unsigned int firstTypeID = image->chunksBegin()->typeID();
	unsigned int chunkCounter = 0;
	for (data::Image::ChunkIterator ci = image->chunksBegin();ci != image->chunksEnd(); *ci++)
	{
		chunkCounter++;
		if(not ci->typeID() == firstTypeID) return false;   
	}
	LOG(DataDebug, info) << "chunkCounter: " << chunkCounter;
	return true;
}
	
}} //end namespace
#endif