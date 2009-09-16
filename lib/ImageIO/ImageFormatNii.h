/**  A one line description of the class.  
 *
 * #include "ImageFormatNii.h" <BR>
 * -llib 
 *
 * A longer description.
 *  
 * @see something
 * 
 */
/*
 * ImageFormatNii.h
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 */

#ifndef IMAGEFORMATNII_H_
#define IMAGEFORMATNII_H_

// SYSTEM INCLUDES
//

// PROJECT INCLUDES
//
#include "DataStorage/ImageFormat.h"
// LOCAL INCLUDES
//

// FORWARD REFERENCES
//


class ImageFormatNii : public ImageFormat
{
public:
	// LIFECYCLE

	//
	/*Creator function
	 * */
	static ImageFormat* CreateImageFormatNii(){
		return new ImageFormatNii;};


	/** Default constructor.
	 */

	ImageFormatNii(
	    void);

	/** Copy constructor.
	 *
	 * @param from The value to copy to this object.
	 */
	ImageFormatNii(
	    const ImageFormatNii& from);

	/** Destructor.
	 */
	~ImageFormatNii(
	    void);

	// OPERATORS

	/** Assignment operator.
	 *
	 * @param from The value to assign to this object.
	 *
	 * @return A reference to this object.
	 */
	ImageFormatNii& operator=(
	    const ImageFormatNii& from);

	// OPERATIONS


	virtual bool ImageFormatIsFunny();
	virtual std::string ImageFormatExtensions();
	virtual bool CanHandleThisFile();
	// ACCESS
	// INQUIRY



protected:
private:
	static const std::string FormatID;
	static const bool isRegistered;
	virtual ImageFormat* CreateClone() const;
};


// EXTERNAL REFERENCES
//

#endif /* IMAGEFORMATNII_H_ */

