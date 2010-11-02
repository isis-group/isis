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

namespace isis {

namespace viewer {

ImageHolder::ImageHolder()
	: m_TimeSteps( 0 ), m_currentTimestep( 0 )
{
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
	return setSliceCoordinates(m_ImageVector[m_currentTimestep]->GetDimensions()[0] / 2, m_ImageVector[m_currentTimestep]->GetDimensions()[1] / 2, m_ImageVector[m_currentTimestep]->GetDimensions()[2] / 2);
}

bool ImageHolder::setSliceCoordinates( const int& x, const int& y, const int& z )
{
	m_X = x;
	m_Y = y;
	m_Z = z;
	m_ExtractorVector[m_BiggestElemVec[0]]->SetOutputWholeExtent( x, x, 0, m_ImageVector[m_currentTimestep]->GetDimensions()[1] - 1, 0, m_ImageVector[m_currentTimestep]->GetDimensions()[2] - 1  );
	m_ExtractorVector[m_BiggestElemVec[0]]->Update();
	m_ExtractorVector[m_BiggestElemVec[1]]->SetOutputWholeExtent( 0, m_ImageVector[m_currentTimestep]->GetDimensions()[0] - 1, y, y, 0, m_ImageVector[m_currentTimestep]->GetDimensions()[2] - 1 );
	m_ExtractorVector[m_BiggestElemVec[1]]->Update();
	m_ExtractorVector[m_BiggestElemVec[2]]->SetOutputWholeExtent( 0, m_ImageVector[m_currentTimestep]->GetDimensions()[0] - 1, 0, m_ImageVector[m_currentTimestep]->GetDimensions()[1] - 1, z, z );
	m_ExtractorVector[m_BiggestElemVec[2]]->Update();
	return true;
}

void ImageHolder::setUpPipe()
{
	//axial
	m_ExtractAxial->SetInput( m_ImageVector[m_currentTimestep] );
	m_MapperAxial->SetInput( m_ExtractAxial->GetOutput() );
	m_ActorAxial->SetMapper( m_MapperAxial );
	m_ActorAxial->GetProperty()->SetInterpolationToFlat();
	m_ActorAxial->SetScale( m_ImageVector[m_currentTimestep]->GetSpacing()[0], m_ImageVector[m_currentTimestep]->GetSpacing()[1], m_ImageVector[m_currentTimestep]->GetSpacing()[2] );
	m_ActorAxial->SetUserMatrix( m_MatrixHandler.getAxialMatrix1() );

	//sagittal
	m_ExtractSagittal->SetInput( m_ImageVector[m_currentTimestep] );
	m_MapperSagittal->SetInput( m_ExtractSagittal->GetOutput() );
	m_ActorSagittal->SetMapper( m_MapperSagittal );
	m_ActorSagittal->GetProperty()->SetInterpolationToFlat();
	m_ActorSagittal->SetScale( m_ImageVector[m_currentTimestep]->GetSpacing()[0], m_ImageVector[m_currentTimestep]->GetSpacing()[1], m_ImageVector[m_currentTimestep]->GetSpacing()[2] );
	m_ActorSagittal->SetUserMatrix( m_MatrixHandler.getSagittalMatrix1() );

	//coronal
	m_ExtractCoronal->SetInput( m_ImageVector[m_currentTimestep] );
	m_MapperCoronal->SetInput( m_ExtractCoronal->GetOutput() );
	m_ActorCoronal->SetMapper( m_MapperCoronal );
	m_ActorCoronal->GetProperty()->SetInterpolationToFlat();
	m_ActorCoronal->SetScale( m_ImageVector[m_currentTimestep]->GetSpacing()[0], m_ImageVector[m_currentTimestep]->GetSpacing()[1], m_ImageVector[m_currentTimestep]->GetSpacing()[2] );
	m_ActorCoronal->SetUserMatrix( m_MatrixHandler.getCoronalMatrix1() );
}

void ImageHolder::setImages( boost::shared_ptr<isis::data::Image> isisImg,  std::vector<vtkSmartPointer<vtkImageData> >imgVec )
{
	m_ImageVector = imgVec;
	LOG( Runtime, info ) << "Image contains " << m_ImageVector.size() << " timesteps.";
	m_TimeSteps = m_ImageVector.size();
	m_ISISImage = isisImg;
	isis::util::TypeReference min, max;
	m_ISISImage->getMinMax( min, max );
	m_Min = min->as<double>();
	m_Max = max->as<double>();
	LOG( Runtime, info ) << "Image minimum: " << min << "; Image maximum: " << max;
	m_readVec = m_ISISImage->getProperty<isis::util::fvector4>("readVec");
	m_phaseVec = m_ISISImage->getProperty<isis::util::fvector4>("phaseVec");
	m_sliceVec = m_ISISImage->getProperty<isis::util::fvector4>("sliceVec");
	LOG( Runtime, info ) << "readVector: " << m_readVec;
	LOG( Runtime, info ) << "phaseVector: " << m_phaseVec;
	LOG( Runtime, info ) << "sliceVector: " << m_sliceVec;
	m_MatrixHandler.setVectors( m_readVec, m_phaseVec, m_sliceVec );
	commonInit();
	createOrientedImages();
	resetSliceCoordinates();
	setUpPipe();

}

bool ImageHolder::createOrientedImages( void )
{
	if( m_MatrixHandler.isRotationMatrix() ) {
		LOG( Runtime, isis::info ) << "Determinant is not 1. Flipping image along slice vector.";
		for( std::vector<vtkSmartPointer<vtkImageData> >::iterator it = m_ImageVector.begin(); it != m_ImageVector.end(); it++ )
		{
			vtkSmartPointer<vtkImageData> tmpImage = *it;
			vtkSmartPointer<vtkImageFlip> flipper = vtkImageFlip::New();
			flipper->SetFilteredAxis(2);
			flipper->SetInput(tmpImage);
			flipper->Update();
			*it = flipper->GetOutput();
		}
	}
	return true;
}


void ImageHolder::commonInit( void  )
{
	LOG( Runtime, info ) << "commonInit.";
	m_ExtractorVector.push_back(m_ExtractSagittal);
	m_ExtractorVector.push_back(m_ExtractCoronal);
	m_ExtractorVector.push_back(m_ExtractAxial);
	m_BiggestElemVec.push_back(getBiggestVecElem<float>(m_readVec));
	m_BiggestElemVec.push_back(getBiggestVecElem<float>(m_phaseVec));
	m_BiggestElemVec.push_back(getBiggestVecElem<float>(m_sliceVec));
}

void ImageHolder::setCurrentTimeStep( const int& timestep )
{
	LOG( Runtime, info ) << "Changing timestep to " << timestep;
	m_currentTimestep = timestep;
	//TODO here we have to call a method that only applies necessary stuff
	setUpPipe();
	setSliceCoordinates(m_X, m_Y, m_Z);
}

}
}
