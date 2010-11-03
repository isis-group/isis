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

#ifndef ISISVIEWER_H
#define ISISVIEWER_H

#include "CoreUtils/log.hpp"
#include "DataStorage/io_factory.hpp"
#include "CoreUtils/vector.hpp"

#include "common.hpp"
#include "ui_isisViewer.h"
#include "viewerCore.hpp"

#include "ImageHolder.hpp"
#include "ViewerInteractor.hpp"

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>
#include <boost/signal.hpp>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

namespace isis {

namespace viewer {

class ViewerInteractor;
class ImageHolder;

class isisViewer
{
public:
	typedef std::map<boost::shared_ptr<data::Image>, std::vector<vtkSmartPointer< vtkImageData > > > ImageMapType;
	isisViewer();
	void init( QVTKWidget *axial, QVTKWidget *sagittal, QVTKWidget *coronal );
	void resetCam();
	void sliceChanged(const int&, const int&, const int&);
	void displayIntensity( const int&, const int&, const int& );
	void addImages( const ImageMapType& );
	void UpdateWidgets();

	void changeCurrentTimeStep( int );
	void checkPhysicalChanged( bool );

	//getter methods
	vtkSmartPointer<vtkImageData> getCurrentVTKImagePtr( void ) const { return m_CurrentImagePtr; }
	boost::shared_ptr<ImageHolder> getCurrentImageHolder( void ) const { return m_CurrentImageHolder; }
	std::vector<boost::shared_ptr< ImageHolder > > getImageHolderVector( void ) const { return m_ImageHolderVector; }

	struct Signals {
			boost::signal< void ( const size_t& )> intensityChanged;
			boost::signal< void ( const size_t&, const size_t&, const size_t&, const size_t& )> mousePosChanged;
		} signalList;

private:

	vtkSmartPointer<vtkImageData> m_CurrentImagePtr;
	boost::shared_ptr<ImageHolder> m_CurrentImageHolder;
	std::vector< boost::shared_ptr< ImageHolder > > m_ImageHolderVector;

	vtkSmartPointer<vtkRenderer> m_RendererAxial;
	vtkSmartPointer<vtkRenderer> m_RendererSagittal;
	vtkSmartPointer<vtkRenderer> m_RendererCoronal;

	vtkSmartPointer<vtkRenderWindow> m_WindowAxial;
	vtkSmartPointer<vtkRenderWindow> m_WindowSagittal;
	vtkSmartPointer<vtkRenderWindow> m_WindowCoronal;

	vtkSmartPointer<ViewerInteractor> m_InteractionStyleCoronal;
	vtkSmartPointer<ViewerInteractor> m_InteractionStyleSagittal;
	vtkSmartPointer<ViewerInteractor> m_InteractionStyleAxial;

	vtkSmartPointer<vtkRenderWindowInteractor> m_InteractorAxial;
	vtkSmartPointer<vtkRenderWindowInteractor> m_InteractorSagittal;
	vtkSmartPointer<vtkRenderWindowInteractor> m_InteractorCoronal;

	QVTKWidget *m_AxialWidget;
	QVTKWidget *m_SagittalWidget;
	QVTKWidget *m_CoronalWidget;

	void setUpPipe();
	void loadImages( util::slist& );

};
}}
#endif
	
