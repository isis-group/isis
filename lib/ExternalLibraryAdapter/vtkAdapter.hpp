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
 * Author: Erik Türke, tuerke@cbs.mpg.de, 2009
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
#include <vtkImageImport.h>
#include <vtkImageData.h>
//external includes 
#include "boost/smart_ptr.hpp"
#include <list>

namespace isis { namespace adapter {
    
class VTKAdapter : public vtkImageImport{
    
    typedef vtkImageImport Superclass;   
public:
    
    static std::list<Superclass*> makeVtkImageList(const boost::shared_ptr<data::Image>);
private:   
    //hold the image
    static boost::shared_ptr<data::Image> m_ImageISIS;
protected:
     //should not be loaded directly
    VTKAdapter();
    VTKAdapter(const VTKAdapter&){};  
  
    
    
};
  
    
}}// end namespace 
#endif //VTKADAPTER_HPP_