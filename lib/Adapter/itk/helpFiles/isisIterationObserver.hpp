/****************************************************************
 *
 * Copyright (C) 2009 Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
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
 * Author: Erik Tuerke, tuerke@cbs.mpg.de, 2009
 *
 *****************************************************************/

#ifndef ISISITERATIONOBSERVER_H
#define ISISITERATIONOBSERVER_H

#include "itkCommand.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkLBFGSBOptimizer.h"
#include "itkAmoebaOptimizer.h"
#include <boost/progress.hpp>

using boost::progress_display;
using boost::progress_timer;

namespace isis
{
namespace extitk
{

class IterationObserver : public itk::Command
{
public:

	typedef IterationObserver Self;
	typedef itk::Command Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	void setVerboseStep( const unsigned int &step ) { m_step = step; }
	itkNewMacro( Self );

private:
	unsigned int m_step;
protected:

	IterationObserver() { m_step = 1; }

public:

	void Execute( itk::Object *caller, const itk::EventObject &event ) {
		Execute( ( const itk::Object * ) caller, event );
	}

	void Execute( const itk::Object *object, const itk::EventObject &event ) {
		if ( !itk::IterationEvent().CheckEvent( &event ) ) {
			return;
		}

		if ( const itk::RegularStepGradientDescentOptimizer *optimizer =
				 dynamic_cast<const itk::RegularStepGradientDescentOptimizer *> ( object ) ) {
			if( ! ( optimizer->GetCurrentIteration() % m_step ) ) {
				if ( optimizer->GetCurrentPosition().Size() <= 6 ) {
					std::cout << "\r" << optimizer->GetCurrentIteration() << " = " << optimizer->GetValue() << " : "
							  << optimizer->GetCurrentPosition() << "   " << std::flush;
				} else {
					std::cout << "\r" << optimizer->GetCurrentIteration() << " = " << optimizer->GetValue() <<  std::flush;
				}
			}
		}

		if ( const itk::VersorRigid3DTransformOptimizer *optimizer =
				 dynamic_cast<const itk::VersorRigid3DTransformOptimizer *> ( object ) ) {
			if( ! ( optimizer->GetCurrentIteration() % m_step ) ) {
				std::cout << "\r" << optimizer->GetCurrentIteration() << " = " << optimizer->GetValue() << " : "
						  << optimizer->GetCurrentPosition() << "	" << std::flush;
			}
		}
	}

};

class ProcessUpdate : public itk::Command
{
public:
	typedef ProcessUpdate Self;
	typedef itk::Command Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	itkNewMacro( Self )
	;
	progress_display display;
protected:
	ProcessUpdate() :
		display( 101 ) {
	}
	;
public:
	typedef const itk::ProcessObject *ProcessPointer;

	void Execute(
		itk::Object *caller, const itk::EventObject &event ) {
		Execute( ( const itk::Object * ) caller, event );
	}

	void Execute(
		const itk::Object *object, const itk::EventObject &event ) {
		if ( !( itk::ProgressEvent().CheckEvent( &event ) ) ) {
			return;
		}

		++display;
	}

};
} //end namespace extitk
} //end namespace isis
#endif // ISISITERATIONOBSERVER_H
