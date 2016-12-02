/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2016  <copyright holder> <email>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef ISISITKIMAGE_H
#define ISISITKIMAGE_H

#include <itkImage.h>
#include "../../data/image.hpp"

namespace isis{
namespace itk4{
namespace _internal{
	template<typename TYPE> class ContainerAdapter:public itk::Image<TYPE,4>::PixelContainer{
	public:
		typedef ContainerAdapter<TYPE>                      Self;
		typedef typename itk::Image<TYPE,4>::PixelContainer Superclass;
		typedef itk::SmartPointer< Self >                   Pointer;
		typedef itk::SmartPointer< const Self >             ConstPointer;

		/** Method for creation through the object factory. */
		itkNewMacro(Self);
	};
}

template<typename TYPE> class IsisITKImage : public itk::Image<TYPE,4>
{
public:
	IsisITKImage(data::TypedChunk<TYPE> &src){
		auto container=_internal::ContainerAdapter<TYPE>::New();
		container.SetImportPointer(src.begin(),src.getVolume(),false);
		
		itk::Image<TYPE,4>::SetPixelContainer(container);
	}
};

}
}

#endif // ISISITKIMAGE_H
