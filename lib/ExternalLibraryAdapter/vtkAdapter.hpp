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
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2009
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

#include "ExternalLibraryAdapterBase.hpp"

//vtk includes
#include <vtkImageData.h>
#include <vtkImageImport.h>
#include <vtkSmartPointer.h>

//TODO chunk handling

namespace isis { namespace adapter {
    
/**
  * VTKAdapter is able of taking an isis image object and return a vector of vtkSmartpointers on vtkImageData objects.
  */
class vtkAdapter : public _internal::ExternalLibraryAdapterBase{
public:
	typedef std::vector< vtkSmartPointer< vtkImageData > > ImageVector;
	
	/**
	* Gets a std::vector of vtkSmartpointers on vtkImageData objects.
	*/
	static ImageVector makeVtkImageDataList(const boost::shared_ptr<data::Image>);
private:   
	boost::shared_ptr<data::Image> m_ImageISIS;
protected:
	//should not be loaded directly
	vtkAdapter(const boost::shared_ptr<data::Image>);
	vtkAdapter(const vtkAdapter&){};  
private:
	ImageVector m_vtkImageDataVector;
		
};
  

    
}}// end namespace 
#endif //VTKADAPTER_HPP_