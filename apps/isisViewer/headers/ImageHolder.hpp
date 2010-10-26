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

#include "CoreUtils/vector.hpp"
#include "DataStorage/image.hpp"
#include "isisViewer.hpp"

#include <cmath>

class isisViewer;

class ImageHolder
{

public:
	ImageHolder();

	void setImage( vtkImageData*, boost::shared_ptr<isis::data::Image> );
	void setPtrToViewer( const boost::shared_ptr<isisViewer> ptr ) { m_PtrToViewer = ptr; }

	void setReadVec( const isis::util::fvector4& read ) { m_readVec = read; }
	void setPhaseVec( const isis::util::fvector4& phase ) { m_phaseVec = phase; }
	void setSliceVec( const isis::util::fvector4& slice ) { m_sliceVec = slice; }

	void setPhysical( const bool& phy ) { m_Physical = phy; }

	isis::util::fvector4 getReadVec() const { return m_readVec; }
	isis::util::fvector4 getPhaseVec() const { return m_phaseVec; }
	isis::util::fvector4 getSliceVec() const { return m_sliceVec; }

	vtkImageData* getVTKImageData() const { return m_Image; }
	boost::shared_ptr<isis::data::Image> getISISImage() const { return m_ISISImage; }

	bool setSliceCoordinates (const int&, const int&, const int& );
	bool resetSliceCoordinates( void );

	vtkActor* getActorAxial() const { return m_ActorAxial; }
	vtkActor* getActorSagittal() const { return m_ActorSagittal; }
	vtkActor* getActorCoronal() const { return m_ActorCoronal; }


private:
	vtkImageData* m_Image;
	boost::shared_ptr<isis::data::Image> m_ISISImage;
	boost::shared_ptr<isisViewer> m_PtrToViewer;
	vtkImageClip* m_ExtractAxial;
	vtkImageClip* m_ExtractSagittal;
	vtkImageClip* m_ExtractCoronal;
	vtkDataSetMapper* m_MapperAxial;
	vtkDataSetMapper* m_MapperSagittal;
	vtkDataSetMapper* m_MapperCoronal;
	vtkActor* m_ActorAxial;
	vtkActor* m_ActorSagittal;
	vtkActor* m_ActorCoronal;

	static const double orientSagittal[];
	static const double orientCoronal[];
	static const double orientAxial[];

	unsigned int m_SliceAxial;
	unsigned int m_SliceSagittal;
	unsigned int m_SliceCoronal;

	isis::util::fvector4 m_readVec;
	isis::util::fvector4 m_phaseVec;
	isis::util::fvector4 m_sliceVec;
	double m_RotX;
	double m_RotY;
	double m_RotZ;

	bool m_Physical;
	void setUpPipe();

};


#endif /* IMAGEHOLDER_HPP_ */
