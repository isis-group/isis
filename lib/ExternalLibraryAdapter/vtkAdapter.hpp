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
 *  Created on: Mar,24 2009
 *      Author: tuerke	
 ******************************************************************/

#ifndef VTKADAPTER_HPP_
#define VTKADAPTER_HPP_

//local includes
#include "DataStorage/image.hpp"
#include "CoreUtils/log.hpp"
#include "CoreUtils/common.hpp"

//vtk includes
#include <vtkImageData.h>
#include <vtkImageImport.h>

//external includes 
#include "boost/smart_ptr.hpp"
#include <list>

namespace isis { namespace adapter {
    
/**
  * VTKAdapter is able of taking an isis Image object and return either a list of vtkImageData* or vtkImageImport*.
  */
class VTKAdapter{
public:
  /**
    * Gets a std::list of vtkImageImport*.
    */
    static std::list<vtkImageImport*> makeVtkImageImportList(const boost::shared_ptr<data::Image>);
  /**
    * Gets a std::list of vtkImageData*.
    */
    static std::list<vtkImageData*> makeVtkImageDataList(const boost::shared_ptr<data::Image>);
private:   
    
    static bool checkChunkDataType(const boost::shared_ptr<data::Image>);
    
protected:
     //should not be loaded directly
    VTKAdapter(const boost::shared_ptr<data::Image>);
    VTKAdapter(const VTKAdapter&){};  
  
private:
    boost::shared_ptr<data::Image> m_ImageISIS;
    std::list<vtkImageData*> m_vtkImageDataList;
    std::list<vtkImageImport*> m_vtkImageImportList;
    
};
  

    
}}// end namespace 
#endif //VTKADAPTER_HPP_