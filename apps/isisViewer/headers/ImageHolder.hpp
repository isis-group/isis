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

#include "CoreUtils/vector.hpp"
#include "DataStorage/image.hpp"
#include "ViewControl.hpp"
#include "MatrixHandler.hpp"

#include <cmath>
#include <vector>

namespace isis
{

namespace viewer
{

class ViewControl;

class ImageHolder
{

public:
	ImageHolder();

	void setImages( util::PropertyMap, std::vector<vtkSmartPointer<vtkImageData> > );
	void setPtrToViewer( const boost::shared_ptr<ViewControl> ptr ) { m_PtrToViewer = ptr; }

	void setReadVec( const isis::util::fvector4 &read ) { m_readVec = read; }
	void setPhaseVec( const isis::util::fvector4 &phase ) { m_phaseVec = phase; }
	void setSliceVec( const isis::util::fvector4 &slice ) { m_sliceVec = slice; }

	isis::util::fvector4 getReadVec() const { return m_readVec; }
	isis::util::fvector4 getPhaseVec() const { return m_phaseVec; }
	isis::util::fvector4 getSliceVec() const { return m_sliceVec; }

	vtkImageData *getVTKImageData() const { return m_ImageVector[m_currentTimestep]; }
	const util::PropertyMap &getISISImage() const { return m_PropertyMap; }

	bool setSliceCoordinates ( const int &, const int &, const int & );
	void setCurrentTimeStep( const int & );
	void setPhysical( bool physical ) {
		m_Physical = physical;
		setCurrentTimeStep( m_currentTimestep );
	}
	bool resetSliceCoordinates( void );

	const int getCurrentTimeStep() const { return m_currentTimestep; }
	const unsigned int getNumberOfTimesteps( void ) const { return m_TimeSteps; }

private:
	MatrixHandler m_MatrixHandler;
	data::Image m_Image;
	util::PropertyMap m_PropertyMap;
	boost::shared_ptr<ViewControl> m_PtrToViewer;
	

	bool m_Physical;
	unsigned int m_TimeSteps;
	unsigned int m_currentTimestep;

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

}
}
#endif /* IMAGEHOLDER_HPP_ */
