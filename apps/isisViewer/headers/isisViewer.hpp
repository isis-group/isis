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

#include "ui_isisViewer.h"
#include "viewerCore.hpp"

#include "ImageHolder.hpp"
#include "ViewerInteractor.hpp"


#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

class ViewerInteractor;
class ImageHolder;

class isisViewer : public QMainWindow
{
	Q_OBJECT

public:
	isisViewer( const isis::util::slist&, QMainWindow *parent = 0 );
	void resetCam();

	void sliceChanged(const int&, const int&, const int&);

	void displayIntensity( const int&, const int&, const int& );

	vtkImageData* m_CurrentImagePtr;
	boost::shared_ptr<ImageHolder> m_CurrentImageHolder;
private slots:
	void valueChangedSagittal( int );
	void valueChangedCoronal( int );
	void valueChangedAxial( int );
	
signals:
	void clicked( bool );
	void valueChanged( int );

private:
	Ui::isisViewer ui;
	std::vector< boost::shared_ptr< ImageHolder > > m_ImageVector;
	void setUpPipe();


	vtkRenderer* m_RendererAxial;
	vtkRenderer* m_RendererSagittal;
	vtkRenderer* m_RendererCoronal;

	vtkRenderWindow* m_WindowAxial;
	vtkRenderWindow* m_WindowSagittal;
	vtkRenderWindow* m_WindowCoronal;

	ViewerInteractor* m_InteractionStyleCoronal;
	ViewerInteractor* m_InteractionStyleSagittal;
	ViewerInteractor* m_InteractionStyleAxial;

	vtkRenderWindowInteractor* m_InteractorAxial;
	vtkRenderWindowInteractor* m_InteractorSagittal;
	vtkRenderWindowInteractor* m_InteractorCoronal;

};

#endif
	
