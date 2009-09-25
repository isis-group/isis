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

namespace isis{ namespace image_io{ 

class ImageFormat_Nifti : public isis::data::FileFormat
{
public:
	// OPERATIONS

	std::string name();
	std::string suffixes();
	std::string dialects();
	bool tainted(){return false;}//internal plugins are not tainted
	isis::data::ChunkList load(std::string filename,std::string dialect);
	bool save(const isis::data::ChunkList &chunks,std::string filename,std::string dialect);
	// ACCESS
	// INQUIRY

};
}}

isis::data::FileFormat* factory(){
  return new isis::image_io::ImageFormat_Nifti();
}

// EXTERNAL REFERENCES
//

#endif /* IMAGEFORMATNII_H_ */

