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

#ifndef VIEWERINTERACTOR_HPP_
#define VIEWERINTERACTOR_HPP_

#include "ViewControl.hpp"

#include <vtkUnsignedCharArray.h>

#include <vtkInteractorStyle.h>
#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkCellPicker.h>

namespace isis
{

namespace viewer
{

class ViewControl;

class ViewerInteractor : public vtkInteractorStyle
{
private:

	int StartPosition[2];
	int EndPosition[2];
	int Moving;
	int Slicing;

	ViewControl *m_ViewerPtr;
	vtkRenderer *m_Renderer;

	vtkUnsignedCharArray *PixelArray;

	vtkCellPicker *m_Picker;

	double MotionFactor;

public:
	virtual void OnMouseMove();
	virtual void OnMiddleButtonUp();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();
	virtual void OnLeftButtonUp();
	virtual void OnLeftButtonDown();
	virtual void Zoom();
	ViewerInteractor( ViewControl *, vtkRenderer * );
	vtkSetMacro( MotionFactor, double );
	vtkGetMacro( MotionFactor, double );

};

}
}
#endif /* VIEWERINTERACTOR_HPP_ */

