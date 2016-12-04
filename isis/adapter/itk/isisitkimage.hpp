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
#include "../../data/chunk.hpp"
#include <itkImportImageContainer.h>
#include "common.hpp"



namespace isis{
namespace itk4{
namespace _internal{
	

class Adapter : public itk::ImportImageContainer<size_t,float>, data::ValueArray<float>{
public:
	typedef Adapter Self;
	typedef itk::ImportImageContainer<size_t,float> Superclass;
	typedef itk::SmartPointer< Self >       Pointer;
	typedef itk::SmartPointer< const Self > ConstPointer;

	void SetContainerManageMemory (bool _arg)override{
		LOG_IF(_arg,Debug,warning) << "Ignoring command to manage container memory";
	}
	bool GetContainerManageMemory () const override{
		return false;
		
	}
 	void ContainerManageMemoryOn ()override{
		
	}
	void ContainerManageMemoryOff ()override{
		
	}
	
	  /** Method for creation through the object factory. */
	itkNewMacro(Self);
	
	void setSource(const data::ValueArray<float> &src){//@todo should not be const
		data::ValueArray<float>::operator=(src);
		SetSize(src.getLength());
		SetCapacity(src.getLength());
	}
	
protected:
	float* 	AllocateElements(size_t size, bool /*UseDefaultConstructor*/) const override{
		LOG_IF(size,Debug,warning) << "Ignoring command to allocate elements";
	}
	void DeallocateManagedMemory ()override{
		LOG(Debug,warning) << "Deallocating ValueArray";
		SetSize(0);
	}
};

}

class IsisITKChunk : public itk::Image<float,4>
{
public:
	IsisITKChunk(const data::TypedChunk<float> &src){ //@todo should not be const
		itk::Image<float,4>::IndexType start{0,0,0,0};
		itk::Image<float,4>::SizeType  size{src.getDimSize(0),src.getDimSize(1),src.getDimSize(2),src.getDimSize(3)};
		itk::Image<float,4>::RegionType region;
		
		region.SetSize( size );
		region.SetIndex( start );

		auto container=_internal::Adapter::New();
		container->setSource(src.getValueArray<float>());
		
		itk::Image<float,4>::SetPixelContainer(container);
		itk::Image<float,4>::SetRegions( region );
		itk::Image<float,4>::ComputeOffsetTable();
		
	}
};

}
}

#endif // ISISITKIMAGE_H
