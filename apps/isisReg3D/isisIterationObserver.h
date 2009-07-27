//
// C++ Interface: isisIterationObserver
//
// Description:
//
//
// Author: Thomas Proeger <proeger@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ISISISISITERATIONOBSERVER_H
#define ISISISISITERATIONOBSERVER_H

#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkCommand.h"

namespace isis {

/**
 * This special implementation of an observer will observe the iteration
 * steps of a VersorRigid3DTransformOptimizer.
 *
 * @author Thomas Proeger <proeger@cbs.mpg.de>
*/
class IterationObserver : public itk::Command {
public:

    // ITK default macros
    typedef IterationObserver 	Self;
    typedef itk::Command			Superlass;
	typedef itk::SmartPointer<Self>	Pointer;

	itkNewMacro(Self);

    virtual void Execute(const itk::Object* caller,
						 const itk::EventObject& event);
    virtual void Execute(itk::Object* caller,
						 const itk::EventObject& event);

protected:

  	typedef itk::VersorRigid3DTransformOptimizer OptimizerType;
	typedef const OptimizerType* OptimizerPointer;

    IterationObserver();
    ~IterationObserver() {}

};

}

#endif
