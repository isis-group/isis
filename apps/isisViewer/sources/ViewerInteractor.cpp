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

#include "ViewerInteractor.hpp"
#include "vtkCallbackCommand.h"

namespace isis {

namespace viewer {

ViewerInteractor::ViewerInteractor( ViewControl* viewer, vtkRenderer* renderer )
	: m_ViewerPtr( viewer ),
	  m_Renderer( renderer )
{
	MotionFactor = 2;
	this->StartPosition[0] = this->StartPosition[1] = 0;
	this->EndPosition[0] = this->EndPosition[1] = 0;
	this->Moving = 0;
	this->PixelArray = vtkUnsignedCharArray::New();
	this->m_Picker = vtkCellPicker::New();
	this->SetCurrentRenderer( m_Renderer );
	this->SetPickColor(1,0,0);

}


void ViewerInteractor::OnRightButtonUp()
{
	m_ViewerPtr->resetCam();
}

void ViewerInteractor::OnLeftButtonDown()
{
	 if (!this->Interactor)
	{
		 return;
	}

	 if( m_Picker->Pick(this->GetInteractor()->GetEventPosition()[0],
	 							this->GetInteractor()->GetEventPosition()[1],
	 							0,
	 							this->CurrentRenderer))
	{
		double ptMapped[3];
		m_Picker->GetMapperPosition(ptMapped);
		m_ViewerPtr->sliceChanged( static_cast<int>(ptMapped[0]), static_cast<int>(ptMapped[1]), static_cast<int>(ptMapped[2]) );
	}
	this->Moving = 1;

}

void ViewerInteractor::OnMouseMove()
{
	if (!this->Moving)
	{
		if( m_Picker->Pick(this->GetInteractor()->GetEventPosition()[0],
							this->GetInteractor()->GetEventPosition()[1],
							0,
							this->CurrentRenderer))
		{
			double ptMapped[3];
			m_Picker->GetMapperPosition(ptMapped);
			m_ViewerPtr->displayIntensity( static_cast<int>(ptMapped[0]), static_cast<int>(ptMapped[1]), static_cast<int>(ptMapped[2]) );
		}

	} else {
		if( m_Picker->Pick(this->GetInteractor()->GetEventPosition()[0],
									this->GetInteractor()->GetEventPosition()[1],
									0,
									this->CurrentRenderer))
		{
			double ptMapped[3];
			m_Picker->GetMapperPosition(ptMapped);
			m_ViewerPtr->displayIntensity( static_cast<int>(ptMapped[0]), static_cast<int>(ptMapped[1]), static_cast<int>(ptMapped[2]) );
			m_ViewerPtr->sliceChanged( static_cast<int>(ptMapped[0]), static_cast<int>(ptMapped[1]), static_cast<int>(ptMapped[2]) );
		}

	}

	if (!this->Interactor || !this->Moving)
	{
		return;
	}
}

void ViewerInteractor::OnLeftButtonUp()
{
  if (!this->Interactor || !this->Moving)
    {
    return;
    }

  if (   (this->StartPosition[0] != this->EndPosition[0])
      || (this->StartPosition[1] != this->EndPosition[1]) )
    {
    this->Zoom();
    }
  this->Moving = 0;
}
void ViewerInteractor::OnMouseWheelForward() 
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->GrabFocus(this->EventCallbackCommand);
  this->StartDolly();
  double factor = this->MotionFactor * 0.2 * this->MouseWheelMotionFactor;
  this->Dolly(pow(1.1, factor));
  this->EndDolly();
  this->ReleaseFocus();
}
void ViewerInteractor::OnMouseWheelBackward()
{
  this->FindPokedRenderer(this->Interactor->GetEventPosition()[0], 
                          this->Interactor->GetEventPosition()[1]);
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  this->GrabFocus(this->EventCallbackCommand);
  this->StartDolly();
  double factor = this->MotionFactor * -0.2 * this->MouseWheelMotionFactor;
  this->Dolly(pow(1.1, factor));
  this->EndDolly();
  this->ReleaseFocus();
}


void ViewerInteractor::Dolly(double factor)
{
	std::cout << "factor: " << factor << std::endl;
  if (this->CurrentRenderer == NULL)
    {
    return;
    }
  
  vtkCamera *camera = this->CurrentRenderer->GetActiveCamera();
  if (camera->GetParallelProjection())
    {
    camera->SetParallelScale(camera->GetParallelScale() / factor);
    }
  else
    {
    camera->Dolly(factor);
    if (this->AutoAdjustCameraClippingRange)
      {
      this->CurrentRenderer->ResetCameraClippingRange();
      }
    }
  
  if (this->Interactor->GetLightFollowCamera()) 
    {
    this->CurrentRenderer->UpdateLightsGeometryToFollowCamera();
    }
  
  this->Interactor->Render();
}
}}
