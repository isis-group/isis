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
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2010
 *
 * vtkAdapter.cpp
 *
 * Description:
 *
 *  Created on: Mar,30 2010
 *      Author: tuerke	
 ******************************************************************/
#ifndef ITKADAPTER_HPP_
#define ITKADAPTER_HPP_

#include "DataStorage/image.hpp"
#include "CoreUtils/log.hpp"

//external includes 
#include <boost/shared_ptr.hpp>

//itk includes
#include <itkSmartPointer.h>
#include <itkImage.h>
#include <itkImportImageContainer.h>

namespace isis{ namespace adapter {

/**
  * ITKAdapter is capable of taking an isis image object and return an itkImage object.
  */

class itkAdapter {
public:
	template<typename T,unsigned short dim> static itk::SmartPointer<itk::Image<T,dim> >
		makeItkImageObject(const boost::shared_ptr<data::Image> src) {
		
		typedef itk::Image<T, dim> MyImageType;    
		itkAdapter* myAdapter = new itkAdapter(src);
		typename MyImageType::Pointer itkImage = MyImageType::New();
		typename MyImageType::SpacingType itkSpacing;
		typename itk::ImportImageContainer<unsigned long, T>::Pointer importer = itk::ImportImageContainer<unsigned long, T>::New();
		const util::fvector4 dimensions(myAdapter->m_ImageISIS->sizeToVector());
		const util::fvector4 indexOrigin(myAdapter->m_ImageISIS->getProperty<util::fvector4>("indexOrigin"));
		const util::fvector4 spacing(myAdapter->m_ImageISIS->getProperty<util::fvector4>("voxelSize"));
		itkSpacing = (spacing[0], spacing[1], spacing[2]);
		importer->Initialize();
		importer->Reserve(dimensions[0]*dimensions[1]*dimensions[2]*dimensions[3]);
						
		itkImage->SetSpacing(itkSpacing);
		itkImage->SetPixelContainer(importer);
		return itkImage;
	};
	
protected:
	//should not be loaded directly
	itkAdapter(const boost::shared_ptr<data::Image>);
	itkAdapter(const itkAdapter&){};  
private:
	boost::shared_ptr<data::Image> m_ImageISIS;
		
};

}}// end namespace

#endif
