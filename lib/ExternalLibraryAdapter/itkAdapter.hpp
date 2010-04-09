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
 *  Created on: Mar,30 2009
 *      Author: tuerke	
 ******************************************************************/
#ifndef ITKADAPTER_HPP_
#define ITKADAPTER_HPP_

#include "ExternalLibraryAdapterBase.hpp"

//itk includes
#include <itkSmartPointer.h>
#include <itkImage.h>

namespace isis{ namespace adapter {

/**
  * ITKAdapter is capable of taking an isis image object and return an itkImage object.
  */

class itkAdapter : public _internal::ExternalLibraryAdapterBase  {
public:
	template<typename T,unsigned short dim> static itk::SmartPointer<itk::Image<T,dim> > 
		makeItkImage(const boost::shared_ptr<data::Image>);
	
protected:
	//should not be loaded directly
	itkAdapter(const boost::shared_ptr<data::Image>);
	itkAdapter(const itkAdapter&){};  
};



}}// end namespace
#endif
