#ifndef ISISITERATIONOBSERVER_H
#define ISISITERATIONOBSERVER_H


#include "itkCommand.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkVersorRigid3DTransformOptimizer.h"
#include "itkLBFGSBOptimizer.h"
#include "itkAmoebaOptimizer.h"


namespace isis
{


class IterationObserver : public itk::Command
{
public:
        typedef IterationObserver Self;
        typedef itk::Command Superclass;
        typedef itk::SmartPointer<Self> Pointer;
	itkNewMacro( Self );
protected:

        IterationObserver() {};

public:

        void Execute ( itk::Object *caller, const itk::EventObject & event ) {
                Execute ( ( const itk::Object * ) caller, event );
        }

        void Execute ( const itk::Object * object, const  itk::EventObject & event ) {
		if ( ! itk::IterationEvent().CheckEvent ( &event ) ) {
                        return;
			}
                if (const itk::RegularStepGradientDescentOptimizer* optimizer = dynamic_cast<const itk::RegularStepGradientDescentOptimizer*> ( object ) )
		{
			
                        std::cout << optimizer->GetCurrentIteration() << " = ";
                        std::cout << optimizer->GetValue() << " : ";
                        std::cout << optimizer->GetCurrentPosition() << std::endl;
		}
		if (const itk::VersorRigid3DTransformOptimizer* optimizer = dynamic_cast<const itk::VersorRigid3DTransformOptimizer*> ( object ) )
		{
			std::cout << optimizer->GetCurrentIteration() << " = ";
                        std::cout << optimizer->GetValue() << " : ";
			std::cout << optimizer->GetCurrentPosition() << std::endl;
		}
		/*
		if (const itk::LBFGSBOptimizer* optimizer = dynamic_cast<const itk::LBFGSBOptimizer*> ( object ) )
		{
			//std::cout << optimizer->GetCurrentIteration() << " = ";
                        //std::cout << optimizer->GetValue() << std::endl;
			//std::cout << optimizer->GetCurrentPosition() << " : ";
		}*/
		
		
		
	}
	
};


} //end namespace isis
#endif // ISISITERATIONOBSERVER_H
