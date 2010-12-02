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

namespace isis
{

namespace viewer
{

ImageHolder::ImageHolder()
	: m_TimeSteps( 0 ),
	  m_currentTimestep( 0 ),
	  m_Physical( false )
{
	m_ExtractAxial = vtkImageClip::New();
	m_ExtractSagittal = vtkImageClip::New();
	m_ExtractCoronal = vtkImageClip::New();
	m_TrivialProducerAxial = vtkTrivialProducer::New();
	m_TrivialProducerSagittal = vtkTrivialProducer::New();
	m_TrivialProducerCoronal = vtkTrivialProducer::New();
	m_MapperAxial = vtkDataSetMapper::New();
	m_MapperSagittal = vtkDataSetMapper::New();
	m_MapperCoronal = vtkDataSetMapper::New();
	m_ActorAxial = vtkActor::New();
	m_ActorSagittal = vtkActor::New();
	m_ActorCoronal = vtkActor::New();
}

bool ImageHolder::resetSliceCoordinates( void )
{
	LOG( Runtime, info ) << "Resetting slice coordinates.";
	setSliceCoordinates( m_ImageVector[m_currentTimestep]->GetDimensions()[0] / 2, m_ImageVector[m_currentTimestep]->GetDimensions()[1] / 2, m_ImageVector[m_currentTimestep]->GetDimensions()[2] / 2 );
}

bool ImageHolder::setSliceCoordinates( const int &x, const int &y, const int &z )
{
	LOG( Runtime, info ) << "Setting slice coordinates to " << x << ", " << y << ", " << z;
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
//TODO this method needs a more effective approach
void ImageHolder::setUpPipe()
{
	LOG( Runtime, info ) << "ImageHolder::setUpPipe";
	//axial
	m_ExtractAxial->SetInput( m_ImageVector[m_currentTimestep] );
	m_MapperAxial->SetInput( m_ExtractAxial->GetOutput() );
	m_ActorAxial->SetMapper( m_MapperAxial );
	m_ActorAxial->GetProperty()->SetInterpolationToFlat();
	m_ActorAxial->SetScale( m_ImageVector[m_currentTimestep]->GetSpacing()[0], m_ImageVector[m_currentTimestep]->GetSpacing()[1], m_ImageVector[m_currentTimestep]->GetSpacing()[2] );

	if ( !m_Physical ) {
		m_ActorAxial->SetUserMatrix( m_MatrixHandler.getAxialMatrix1() );
		m_ActorAxial->SetPosition( m_pseudoOrigin[0], m_pseudoOrigin[1], m_pseudoOrigin[2] );
	} else {
		m_ActorAxial->SetUserMatrix( m_MatrixHandler.getAxialMatrix() );
		//      m_ActorAxial->SetPosition( m_transformedOrigin[0] * 2, m_transformedOrigin[1] * 2, m_transformedOrigin[2] * 2 );
	}

	//sagittal
	m_ExtractSagittal->SetInput( m_ImageVector[m_currentTimestep] );
	m_MapperSagittal->SetInput( m_ExtractSagittal->GetOutput() );
	m_ActorSagittal->SetMapper( m_MapperSagittal );
	m_ActorSagittal->GetProperty()->SetInterpolationToFlat();
	m_ActorSagittal->SetScale( m_ImageVector[m_currentTimestep]->GetSpacing()[0], m_ImageVector[m_currentTimestep]->GetSpacing()[1], m_ImageVector[m_currentTimestep]->GetSpacing()[2] );

	if ( !m_Physical ) {
		m_ActorSagittal->SetUserMatrix( m_MatrixHandler.getSagittalMatrix1() );
		m_ActorSagittal->SetPosition( m_pseudoOrigin[0], m_pseudoOrigin[1], m_pseudoOrigin[2] );
	} else {
		m_ActorSagittal->SetUserMatrix( m_MatrixHandler.getSagittalMatrix() );
		//      m_ActorSagittal->SetPosition( m_transformedOrigin[0], m_transformedOrigin[1], m_transformedOrigin[2] );
	}

	//coronal
	m_ExtractCoronal->SetInput( m_ImageVector[m_currentTimestep] );
	m_MapperCoronal->SetInput( m_ExtractCoronal->GetOutput() );
	m_ActorCoronal->SetMapper( m_MapperCoronal );
	m_ActorCoronal->GetProperty()->SetInterpolationToFlat();
	m_ActorCoronal->SetScale( m_ImageVector[m_currentTimestep]->GetSpacing()[0], m_ImageVector[m_currentTimestep]->GetSpacing()[1], m_ImageVector[m_currentTimestep]->GetSpacing()[2] );
	m_ActorCoronal->SetUserMatrix( m_MatrixHandler.getCoronalMatrix1() );

	if ( !m_Physical ) {
		m_ActorCoronal->SetUserMatrix( m_MatrixHandler.getCoronalMatrix1() );
		m_ActorCoronal->SetPosition( m_pseudoOrigin[0], m_pseudoOrigin[1], m_pseudoOrigin[2] );
	} else {
		m_ActorCoronal->SetUserMatrix( m_MatrixHandler.getCoronalMatrix() );
		//      m_ActorCoronal->SetPosition( m_transformedOrigin[0], m_transformedOrigin[1], m_transformedOrigin[2] );
	}
}

void ImageHolder::setImages( util::PropMap propMap,  std::vector<vtkSmartPointer<vtkImageData> >imgVec )
{
	m_ImageVector = imgVec;
	LOG( Runtime, info ) << "Image contains " << m_ImageVector.size() << " timesteps.";
	m_TimeSteps = m_ImageVector.size();
	m_PropMap = propMap;
	isis::util::TypeReference min, max;
#warning check this - the interfave for the conversion does not expect min max anymore
	LOG( Runtime, info ) << "Image minimum: " << min << "; Image maximum: " << max;
	m_ScalingFactor = m_PropMap.propertyValue( "scale" );
	m_Offset = m_PropMap.propertyValue( "offset" );
	m_readVec = m_PropMap.getProperty<isis::util::fvector4>( "readVec" );
	m_phaseVec = m_PropMap.getProperty<isis::util::fvector4>( "phaseVec" );
	m_sliceVec = m_PropMap.getProperty<isis::util::fvector4>( "sliceVec" );
	LOG( Runtime, info ) << "readVector: " << m_readVec;
	LOG( Runtime, info ) << "phaseVector: " << m_phaseVec;
	LOG( Runtime, info ) << "sliceVector: " << m_sliceVec;
	m_MatrixHandler.setVectors( m_readVec, m_phaseVec, m_sliceVec );
	LOG( Runtime, info ) << "spacing[0]: " << m_ImageVector.front()->GetSpacing()[0];
	m_pseudoOrigin = m_MatrixHandler.createPseudoOrigin( m_PropMap.getProperty<util::fvector4>( "imageSize" ), m_PropMap.getProperty<util::fvector4>( "voxelSize" ) );
	m_transformedOrigin = m_MatrixHandler.transformOrigin( m_PropMap.getProperty<util::fvector4>( "indexOrigin" ), m_PropMap.getProperty<util::fvector4>( "voxelSize" ) );
	//TODO debug
	std::cout << "transformedOrigin: " << m_transformedOrigin << std::endl;
	commonInit();
	createOrientedImages();
	setUpPipe();
	resetSliceCoordinates();
}

bool ImageHolder::createOrientedImages( void )
{
	if( m_MatrixHandler.isRotationMatrix() ) {
		LOG( Runtime, isis::info ) << "Determinant is not 1. Flipping image along slice vector.";

		for( std::vector<vtkSmartPointer<vtkImageData> >::iterator it = m_ImageVector.begin(); it != m_ImageVector.end(); it++ ) {
			vtkSmartPointer<vtkImageData> tmpImage = *it;
			tmpImage->SetSpacing( ( *it )->GetSpacing() );
			tmpImage->SetOrigin( ( *it )->GetOrigin() );
			vtkSmartPointer<vtkImageFlip> flipper = vtkImageFlip::New();
			flipper->SetFilteredAxis( 2 );
			flipper->SetInput( tmpImage );
			flipper->Update();
			*it = flipper->GetOutput();
			( *it )->SetSpacing( tmpImage->GetSpacing() );
			( *it )->SetOrigin( tmpImage->GetOrigin() );
		}
	}

	return true;
}


void ImageHolder::commonInit( void  )
{
	LOG( Runtime, info ) << "commonInit.";
	m_ExtractorVector.push_back( m_ExtractSagittal );
	m_ExtractorVector.push_back( m_ExtractCoronal );
	m_ExtractorVector.push_back( m_ExtractAxial );
	m_BiggestElemVec.push_back( getBiggestVecElem<float>( m_readVec ) );
	m_BiggestElemVec.push_back( getBiggestVecElem<float>( m_phaseVec ) );
	m_BiggestElemVec.push_back( getBiggestVecElem<float>( m_sliceVec ) );
}

void ImageHolder::setCurrentTimeStep( const int &timestep )
{
	LOG( Runtime, info ) << "Changing timestep to " << timestep;
	m_currentTimestep = timestep;
	//TODO here we have to call a method that only applies necessary stuff
	setUpPipe();
	setSliceCoordinates( m_X, m_Y, m_Z );
}

}
} // end namespace
