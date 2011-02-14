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

}


//TODO this method needs a more effective approach
void ImageHolder::setUpPipe()
{
	LOG( Runtime, info ) << "ImageHolder::setUpPipe";
	//axial
	
}

void ImageHolder::setImages( util::PropertyMap propMap,  std::vector<vtkSmartPointer<vtkImageData> >imgVec )
{
	m_ImageVector = imgVec;
	LOG( Runtime, info ) << "Image contains " << m_ImageVector.size() << " timesteps.";
	m_TimeSteps = m_ImageVector.size();
	m_PropertyMap = propMap;
	isis::util::TypeReference min, max;
#warning check this - the interfave for the conversion does not expect min max anymore
	LOG( Runtime, info ) << "Image minimum: " << min << "; Image maximum: " << max;
	m_ScalingFactor = m_PropertyMap.propertyValue( "scale" );
	m_Offset = m_PropertyMap.propertyValue( "offset" );
	m_readVec = m_PropertyMap.getPropertyAs<isis::util::fvector4>( "readVec" );
	m_phaseVec = m_PropertyMap.getPropertyAs<isis::util::fvector4>( "phaseVec" );
	m_sliceVec = m_PropertyMap.getPropertyAs<isis::util::fvector4>( "sliceVec" );
	LOG( Runtime, info ) << "readVector: " << m_readVec;
	LOG( Runtime, info ) << "phaseVector: " << m_phaseVec;
	LOG( Runtime, info ) << "sliceVector: " << m_sliceVec;
	m_MatrixHandler.setVectors( m_readVec, m_phaseVec, m_sliceVec );
	LOG( Runtime, info ) << "spacing[0]: " << m_ImageVector.front()->GetSpacing()[0];
	m_pseudoOrigin = m_MatrixHandler.createPseudoOrigin( m_PropertyMap.getPropertyAs<util::fvector4>( "imageSize" ), m_PropertyMap.getPropertyAs<util::fvector4>( "voxelSize" ) );
	m_transformedOrigin = m_MatrixHandler.transformOrigin( m_PropertyMap.getPropertyAs<util::fvector4>( "indexOrigin" ), m_PropertyMap.getPropertyAs<util::fvector4>( "voxelSize" ) );
	//TODO debug
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
}

}
} // end namespace
