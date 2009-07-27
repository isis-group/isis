#include "isisIterationObserver.h"

namespace isis {


  isis::IterationObserver::IterationObserver()
  {
  }


  void IterationObserver::Execute(itk::Object* caller,
				  const itk::EventObject& event)
  {
    Execute( (const itk::Object *) caller, event);
  }

  void IterationObserver::Execute(const itk::Object* caller,
				  const itk::EventObject& event)
  {
    OptimizerPointer optimizer
      = dynamic_cast<OptimizerPointer> (caller);

    // check the event type
    if(! itk::IterationEvent().CheckEvent(& event))
      return;

    // print some informations
    std::cout << "Step: " << optimizer->GetCurrentIteration() << " = "
	      << optimizer->GetValue() << " : "
	      << optimizer->GetCurrentPosition() << std::endl;
  }



}
