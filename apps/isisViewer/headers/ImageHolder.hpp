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

#ifndef IMAGEHOLDER_HPP_
#define IMAGEHOLDER_HPP_

#include <vtkImageData.h>
#include <vtkImageFlip.h>
#include <vtkDataSetMapper.h>
#include <vtkImageMapper.h>
#include <vtkImageClip.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkActor.h>
#include <vtkInteractorStyleRubberBandZoom.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkCamera.h>
#include <vtkMatrix4x4.h>
#include <vtkTransform.h>
#include <vtkSmartPointer.h>
#include <vtkTrivialProducer.h>

#include "CoreUtils/vector.hpp"
#include "DataStorage/image.hpp"
#include "ViewControl.hpp"
#include "MatrixHandler.hpp"

#include <cmath>
#include <vector>

namespace isis {

namespace viewer {

class ViewControl;

class ImageHolder
{

public:
	ImageHolder();

	void setImages( util::PropMap, std::vector<vtkSmartPointer<vtkImageData> >);
	void setPtrToViewer( const boost::shared_ptr<ViewControl> ptr ) { m_PtrToViewer = ptr; }

	void setReadVec( const isis::util::fvector4& read ) { m_readVec = read; }
	void setPhaseVec( const isis::util::fvector4& phase ) { m_phaseVec = phase; }
	void setSliceVec( const isis::util::fvector4& slice ) { m_sliceVec = slice; }

	isis::util::fvector4 getReadVec() const { return m_readVec; }
	isis::util::fvector4 getPhaseVec() const { return m_phaseVec; }
	isis::util::fvector4 getSliceVec() const { return m_sliceVec; }

	vtkImageData* getVTKImageData() const { return m_ImageVector[m_currentTimestep]; }
	const util::PropMap& getISISImage() const { return m_PropMap; }

	bool setSliceCoordinates (const int&, const int&, const int& );
	void setCurrentTimeStep( const int& );
	void setPhysical( bool physical ) {
		m_Physical = physical;
		setCurrentTimeStep(m_currentTimestep);
	}
	bool resetSliceCoordinates( void );

	vtkActor* getActorAxial() const { return m_ActorAxial; }
	vtkActor* getActorSagittal() const { return m_ActorSagittal; }
	vtkActor* getActorCoronal() const { return m_ActorCoronal; }
	const int getCurrentTimeStep() const { return m_currentTimestep; }
	const unsigned int getNumberOfTimesteps( void ) const { return m_TimeSteps; }
	const util::TypeReference getScalingFactor( void ) const { return m_ScalingFactor; }
	const util::TypeReference getOffset( void ) const { return m_Offset; }

private:
	MatrixHandler m_MatrixHandler;
	std::vector<vtkSmartPointer<vtkImageData> > m_ImageVector;
	util::PropMap m_PropMap;
	boost::shared_ptr<ViewControl> m_PtrToViewer;
	vtkSmartPointer<vtkImageClip> m_ExtractAxial;
	vtkSmartPointer<vtkImageClip> m_ExtractSagittal;
	vtkSmartPointer<vtkImageClip> m_ExtractCoronal;
	vtkSmartPointer<vtkTrivialProducer> m_TrivialProducerAxial;
	vtkSmartPointer<vtkTrivialProducer> m_TrivialProducerSagittal;
	vtkSmartPointer<vtkTrivialProducer> m_TrivialProducerCoronal;
	std::vector<vtkSmartPointer<vtkImageClip> > m_ExtractorVector;
	vtkSmartPointer<vtkDataSetMapper> m_MapperAxial;
	vtkSmartPointer<vtkDataSetMapper> m_MapperSagittal;
	vtkSmartPointer<vtkDataSetMapper> m_MapperCoronal;
	vtkSmartPointer<vtkActor> m_ActorAxial;
	vtkSmartPointer<vtkActor> m_ActorSagittal;
	vtkSmartPointer<vtkActor> m_ActorCoronal;

	bool m_Physical;
	unsigned int m_TimeSteps;
	unsigned int m_currentTimestep;
	size_t m_X, m_Y, m_Z;

	util::TypeReference m_ScalingFactor;
	util::TypeReference m_Offset;

	size_t m_Min, m_Max;

	std::vector<size_t> m_BiggestElemVec;

	isis::util::fvector4 m_readVec;
	isis::util::fvector4 m_phaseVec;
	isis::util::fvector4 m_sliceVec;
	util::fvector4 m_pseudoOrigin;
	util::fvector4 m_transformedOrigin;


	void setUpPipe( void );
	bool createOrientedImages( void );
	void commonInit( void );

};

}}
#endif /* IMAGEHOLDER_HPP_ */
