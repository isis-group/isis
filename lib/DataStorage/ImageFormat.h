/**  A one line description of the class.  
 *
 * #include "ImageFormat.h" <BR>
 * -llib 
 *
 * A longer description.
 *  
 * @see something
 * 
 */
/*
 * ImageFormat.h
 *
 *  Created on: Aug 11, 2009
 *      Author: hellrung
 */

#ifndef IMAGEFORMAT_H_
#define IMAGEFORMAT_H_

// SYSTEM INCLUDES
//
#include <typeinfo>
#include <iostream>
// PROJECT INCLUDES
//

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//



class ImageFormat
{
public:
// LIFECYCLE

   /** Default constructor.
    */

	ImageFormat(void);

   /** Copy constructor.
    * 
    * @param from The value to copy to this object.
    */
   ImageFormat(const ImageFormat& from);


   /** Destructor.
    */
   ~ImageFormat(void);

   /*Clone
    * clones the ImageFormat object and makes a runtime check of type safety
    * */
   ImageFormat* Clone() const{
	   ImageFormat* pImageFormat = CreateClone();//call private function doing the job
	   if(typeid(*pImageFormat) == typeid(*this)){
		   //TODO: do something smarter than if-test + LOG
		   return pImageFormat;//all derived classes implements CreateClone(), so everything is fine
	   }
	   else{
		   //TODO: LOG and error handling
	   }

   }



// OPERATORS

   /** Assignment operator.
    *
    * @param from The value to assign to this object.
    *
    * @return A reference to this object.
    */
   ImageFormat&  operator=(const ImageFormat& from);  

// OPERATIONS

   virtual bool ImageFormatIsFunny() = 0;

   virtual std::string ImageFormatExtensions() = 0;

   //TODO: add real interface functions

   virtual bool CanHandleThisFile() = 0;


// ACCESS
// INQUIRY

protected:
private:
   virtual ImageFormat* CreateClone() const = 0;
};

// INLINE METHODS
//

// EXTERNAL REFERENCES
//

#endif /* IMAGEFORMAT_H_ */

