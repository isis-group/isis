/**  A one line description of the class.  
 *
 * #include "ImageFormatDCM.h" <BR>
 * -llib 
 *
 * A longer description.
 *  
 * @see something
 * 
 */
/*
 * ImageFormatDCM.h
 *
 *  Created on: Sep 4, 2009
 *      Author: hellrung
 */

#ifndef IMAGEFORMATDCM_H_
#define IMAGEFORMATDCM_H_

// SYSTEM INCLUDES
//

// PROJECT INCLUDES
//
#include "DataStorage/ImageFormat.h"

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//


class ImageFormatDCM : public ImageFormat
{
public:
	// LIFECYCLE

	/** Default constructor.
	 */

	ImageFormatDCM(
	    void);

	/*just for test purposes*/
	ImageFormatDCM(
	    const int& i) :
		m_int(i) {
		printf("konstruiert mit %d\n", m_int);
	}

	/** Copy constructor.
	 *
	 * @param from The value to copy to this object.
	 */
	ImageFormatDCM(
	    const ImageFormatDCM& from);

	/** Destructor.
	 */
	~ImageFormatDCM(
	    void);

	// OPERATORS

	/** Assignment operator.
	 *
	 * @param from The value to assign to this object.
	 *
	 * @return A reference to this object.
	 */
	ImageFormatDCM& operator=(
	    const ImageFormatDCM& from);

	// OPERATIONS

	/*
	 * Creator function*/
	static ImageFormat* CreateImageFormatDCM(){
		return new ImageFormatDCM;
	}

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
	int m_int;
};

// INLINE METHODS
//

// EXTERNAL REFERENCES
//

#endif /* IMAGEFORMATDCM_H_ */

