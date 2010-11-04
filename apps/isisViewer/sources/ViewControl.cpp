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


#include "ViewControl.hpp"

namespace isis {

namespace viewer {

ViewControl::ViewControl( ) : m_Valid( false )
{
	LOG(Runtime, info ) << "ViewControl::ViewControl";
	m_CurrentImagePtr = vtkImageData::New();

	m_RendererAxial = vtkRenderer::New();
	m_RendererSagittal = vtkRenderer::New();
	m_RendererCoronal = vtkRenderer::New();
}

void ViewControl::init(QVTKWidget *axial, QVTKWidget *sagittal, QVTKWidget *coronal )
{
	m_Valid = true;
	LOG (Runtime, info ) << "ViewControl::init";
	m_AxialWidget = axial;
	m_SagittalWidget = sagittal;
	m_CoronalWidget = coronal;

	m_InteractionStyleAxial = new ViewerInteractor(this, m_RendererAxial);
	m_InteractionStyleSagittal = new ViewerInteractor(this, m_RendererSagittal);
	m_InteractionStyleCoronal = new ViewerInteractor(this, m_RendererCoronal);
	setUpPipe();

	LOG(Runtime, info ) << "Initializing interactors";
	m_AxialWidget->GetInteractor()->Initialize();
	m_SagittalWidget->GetInteractor()->Initialize();
	m_CoronalWidget->GetInteractor()->Initialize();



}

void ViewControl::addImages( const ImageMapType& fileMap )
{
	LOG_IF(!m_Valid, Runtime, error ) << "ViewControl is not valid. Please call the init function prior to adding images.";
	assert(m_Valid);
	LOG(Runtime, info) << "ViewControl::addImages";
	BOOST_FOREACH( ImageMapType::const_reference ref, fileMap )
	{
		boost::shared_ptr< ImageHolder > tmpVec( new ImageHolder );
		tmpVec->setImages( ref.first, ref.second );
		tmpVec->setReadVec( ref.first->getProperty<isis::util::fvector4>("readVec") );
		tmpVec->setPhaseVec( ref.first->getProperty<isis::util::fvector4>("phaseVec") );
		tmpVec->setSliceVec( ref.first->getProperty<isis::util::fvector4>("sliceVec") );
		m_ImageHolderVector.push_back( tmpVec );
	}

	if (!m_ImageHolderVector.empty() ) {
		m_CurrentImageHolder = m_CurrentImageHolder ? m_CurrentImageHolder : m_ImageHolderVector.front();
		m_CurrentImagePtr = m_CurrentImageHolder ? m_CurrentImageHolder->getVTKImageData() : m_ImageHolderVector.front()->getVTKImageData();
		BOOST_FOREACH( std::vector< boost::shared_ptr< ImageHolder > >::const_reference ref, m_ImageHolderVector )
		{
			LOG(Runtime, info) << "Adding actors to renderers";
			m_RendererAxial->AddActor( ref->getActorAxial() );
			m_RendererCoronal->AddActor( ref->getActorCoronal() );
			m_RendererSagittal->AddActor( ref->getActorSagittal() );
		}
	}
	resetCam();
}

void ViewControl::setUpPipe()
{
	LOG( Runtime, info ) << "Setting up the pipe";
	m_AxialWidget->GetInteractor()->SetInteractorStyle( m_InteractionStyleAxial );
	m_SagittalWidget->GetInteractor()->SetInteractorStyle( m_InteractionStyleSagittal );
	m_CoronalWidget->GetInteractor()->SetInteractorStyle( m_InteractionStyleCoronal );

	m_AxialWidget->GetRenderWindow()->SetInteractor( m_AxialWidget->GetInteractor() );
	m_SagittalWidget->GetRenderWindow()->SetInteractor( m_SagittalWidget->GetInteractor() );
	m_CoronalWidget->GetRenderWindow()->SetInteractor( m_CoronalWidget->GetInteractor() );

	m_AxialWidget->GetRenderWindow()->AddRenderer( m_RendererCoronal );
	m_SagittalWidget->GetRenderWindow()->AddRenderer( m_RendererSagittal );
	m_CoronalWidget->GetRenderWindow()->AddRenderer( m_RendererAxial );
}

void ViewControl::resetCam()
{
	LOG(Runtime, info) << "ViewControl::resetCam";
	BOOST_FOREACH( std::vector< boost::shared_ptr< ImageHolder > >::const_reference refImg, m_ImageHolderVector)
	{
		refImg->resetSliceCoordinates();
	}
	UpdateWidgets();
}

void ViewControl::UpdateWidgets()
{
	LOG(Runtime, info ) << "ViewControl::UpdateWidgets";
	m_AxialWidget->update();
	m_SagittalWidget->update();
	m_CoronalWidget->update();
	m_RendererAxial->ResetCamera();
	m_RendererSagittal->ResetCamera();
	m_RendererCoronal->ResetCamera();

}

//gui interactions

void ViewControl::displayIntensity( const int& x, const int& y, const int &z )
{

	const int t = m_CurrentImageHolder->getCurrentTimeStep();
	signalList.mousePosChanged( x, y, z, t );

	switch (m_CurrentImageHolder->getISISImage()->getChunk(x,y,z,t ).typeID() )
	{
	case isis::data::TypePtr<int8_t>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<int8_t>(x, y,z, t));
		break;
	case isis::data::TypePtr<u_int8_t>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<u_int8_t>(x, y,z, t));
		break;
	case isis::data::TypePtr<int16_t>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<int16_t>(x, y,z, t));
		break;
	case isis::data::TypePtr<u_int16_t>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<u_int16_t>(x, y,z, t));
		break;
	case isis::data::TypePtr<int32_t>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<int32_t>(x, y,z, t));
		break;
	case isis::data::TypePtr<u_int32_t>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<u_int32_t>(x, y,z, t));
		break;
	case isis::data::TypePtr<float>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<float>(x, y,z, t));
		break;
	case isis::data::TypePtr<double>::staticID:
		signalList.intensityChanged(m_CurrentImageHolder->getISISImage()->voxel<double>(x, y,z, t));
		break;
	default:
		LOG( Runtime, error )
		<< "Unknown pixel representation with typeid " <<
			m_CurrentImageHolder->getISISImage()->getChunk(x,y,z,t ).typeID() <<
		". Can not display intensity! ";

		signalList.intensityChanged(-1);
	}

}

void ViewControl::sliceChanged( const int& x, const int& y, const int& z)
{
	LOG( Runtime, info ) << "ViewControl::sliceChanged";
	BOOST_FOREACH( std::vector< boost::shared_ptr< ImageHolder > >::const_reference refImg, m_ImageHolderVector)
	{
		if ( not refImg->setSliceCoordinates(x,y,z) ) LOG( Runtime, error ) << "error during setting slicesetting!";
	}
	UpdateWidgets();
}

void ViewControl::changeCurrentTimeStep( int val )
{
	m_CurrentImageHolder->setCurrentTimeStep( val );
	UpdateWidgets();

}

void ViewControl::checkPhysicalChanged( bool physical )
{
	LOG(Runtime, info) << "Setting physical to " << physical;
	BOOST_FOREACH(std::vector< boost::shared_ptr< ImageHolder > >::const_reference ref, m_ImageHolderVector)
	{
		ref->setPhysical( physical );
	}
	UpdateWidgets();
}

}}
