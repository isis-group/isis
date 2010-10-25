/****************************************************************
 *
 *  Copyright (C) 2010 Max Planck Institute for Human Cognitive and Brain Sciences, Leipzig
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

#include "ImageHolder.hpp"

//rotation values for visualization in image space
const double ImageHolder::orientSagittal[] = {0,90,-90};
const double ImageHolder::orientAxial[] = {180,0,0};
const double ImageHolder::orientCoronal[] = {90,0,0};

ImageHolder::ImageHolder()
{
	m_Physical = false;
	m_Image = vtkImageData::New();
	m_ExtractAxial = vtkImageClip::New();
	m_ExtractSagittal = vtkImageClip::New();
	m_ExtractCoronal = vtkImageClip::New();
	m_MapperAxial = vtkDataSetMapper::New();
	m_MapperSagittal = vtkDataSetMapper::New();
	m_MapperCoronal = vtkDataSetMapper::New();
	m_ActorAxial = vtkActor::New();
	m_ActorSagittal = vtkActor::New();
	m_ActorCoronal = vtkActor::New();

}

bool ImageHolder::resetSliceCoordinates( void )
{
	return setSliceCoordinates(m_Image->GetDimensions()[0] / 2, m_Image->GetDimensions()[1] / 2, m_Image->GetDimensions()[2] / 2);
}

bool ImageHolder::setSliceCoordinates( const int& sagittal, const int& coronal, const int& axial )
{
	if( axial <= (m_Image->GetDimensions()[2] - 1))
	{
		//TODO debug
		std::cout << "axial: " << axial << std::endl;
		m_SliceAxial = axial;
		m_ExtractAxial->SetOutputWholeExtent( 0, m_Image->GetDimensions()[0] - 1, 0, m_Image->GetDimensions()[1] - 1, m_SliceAxial, m_SliceAxial );
	} else { return false; }
	if( coronal <= (m_Image->GetDimensions()[1] - 1) )
	{
		//TODO debug
		std::cout << "coronal: " << coronal << std::endl;
		m_SliceCoronal = coronal;
		m_ExtractCoronal->SetOutputWholeExtent( 0, m_Image->GetDimensions()[0] - 1, m_SliceCoronal, m_SliceCoronal, 0, m_Image->GetDimensions()[2] - 1 );
	} else { return false; }
	if( sagittal <= (m_Image->GetDimensions()[0] - 1) )
	{
		//TODO debug
		std::cout << "sagittal: " << sagittal << std::endl;
		m_SliceSagittal = sagittal;
		m_ExtractSagittal->SetOutputWholeExtent( m_SliceSagittal, m_SliceSagittal, 0, m_Image->GetDimensions()[1] - 1, 0, m_Image->GetDimensions()[2] - 1  );
	} else { return false; }

	m_ExtractAxial->Update();
	m_ExtractSagittal->Update();
	m_ExtractCoronal->Update();

	return true;
}

void ImageHolder::setUpPipe()
{
	//axial
	m_ExtractAxial->SetInput( m_Image );
	m_MapperAxial->SetInput( m_ExtractAxial->GetOutput() );
	m_ActorAxial->SetMapper( m_MapperAxial );
	m_ActorAxial->GetProperty()->SetInterpolationToFlat();
	m_ActorAxial->SetScale( m_Image->GetSpacing()[0], m_Image->GetSpacing()[1], m_Image->GetSpacing()[2] );
	m_ActorAxial->SetOrientation( orientAxial[0], orientAxial[1], orientAxial[2] );

	//sagittal
	m_ExtractSagittal->SetInput( m_Image );
	m_MapperSagittal->SetInput( m_ExtractSagittal->GetOutput() );
	m_ActorSagittal->SetMapper( m_MapperSagittal );
	m_ActorSagittal->GetProperty()->SetInterpolationToFlat();
	m_ActorSagittal->SetScale( m_Image->GetSpacing()[0], m_Image->GetSpacing()[1], m_Image->GetSpacing()[2] );
	m_ActorSagittal->SetOrientation( orientSagittal[0], orientSagittal[1], orientSagittal[2] );

	//coronal
	m_ExtractCoronal->SetInput( m_Image );
	m_MapperCoronal->SetInput( m_ExtractCoronal->GetOutput() );
	m_ActorCoronal->SetMapper( m_MapperCoronal );
	m_ActorCoronal->GetProperty()->SetInterpolationToFlat();
	m_ActorCoronal->SetScale( m_Image->GetSpacing()[0], m_Image->GetSpacing()[1], m_Image->GetSpacing()[2] );
	m_ActorCoronal->SetOrientation( orientCoronal[0], orientCoronal[1], orientCoronal[2] );
}

void ImageHolder::setImage( vtkImageData* img, boost::shared_ptr<isis::data::Image> isisImg )
{
	m_Image = img;
	m_ISISImage = isisImg;
	setSliceCoordinates( (m_Image->GetDimensions()[0] / 2), (m_Image->GetDimensions()[1] / 2), (m_Image->GetDimensions()[2] /2) );
	setUpPipe();

}

void ImageHolder::calculateRotations()
{
	m_RotY = atan2(m_readVec[2], sqrt((m_readVec[0]*m_readVec[0])+(m_readVec[1]*m_readVec[1]))) * (180 / M_PI);
	m_RotZ = atan2((m_readVec[1] / cos(m_RotY)), m_readVec[0] / cos(m_RotY)) * (180 / M_PI);
	m_RotX = atan2((m_phaseVec[2] / cos(m_RotY)), m_sliceVec[2] / cos(m_RotY)) * (180 / M_PI);
	//TODO debug
	std::cout << "x: " << m_RotX << std::endl;
	std::cout << "y: " << m_RotY << std::endl;
	std::cout << "z: " << m_RotZ << std::endl;
}


