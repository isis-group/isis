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
#include <DataStorage/io_interface.h>

// LOCAL INCLUDES
//

// FORWARD REFERENCES
//


class ImageFormatNii : public isis::data::FileFormat
{
public:
	// LIFECYCLE

	/** Default constructor.
	 */
	ImageFormatNii(void);
	/** Destructor.
	 */
	~ImageFormatNii(void);


	// OPERATIONS

	std::string name();
	std::string suffixes();
	std::string dialects();
	bool tainted(){return false;}//internal plugins are not tainted
	isis::data::Chunks load(std::string filename,std::string dialect);
	bool save(const isis::data::Chunks &chunks,std::string filename,std::string dialect);
	// ACCESS
	// INQUIRY



protected:
private:

	/** Copy constructor.
	 * @param from The value to copy to this object.
	 */
	ImageFormatNii(const ImageFormatNii& from);


	// OPERATORS

	/** Assignment operator.
	 * @param from The value to assign to this object.
	 * @return A reference to this object.
	 */
	ImageFormatNii& operator=(const ImageFormatNii&);

};

isis::data::FileFormat* factory(){
  return new ImageFormatNii();
}

// EXTERNAL REFERENCES
//

#endif /* IMAGEFORMATNII_H_ */

