/*
 * ImageFormat.cpp
 *
 *  Created on: Aug 11, 2009
 *      Author: hellrung
 */

#include "ImageFormat.h"  // class implemented

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ====================================

ImageFormat::ImageFormat() {
}// ImageFormat

ImageFormat::ImageFormat(
    const ImageFormat& from) {
}// ImageFormat

ImageFormat::~ImageFormat() {
}// ~ImageFormat


//============================= OPERATORS ====================================

ImageFormat&
ImageFormat::operator=(
    const ImageFormat& from) {
	if(this == &from) {
		return *this;
	}

	return *this;

}// =

//============================= OPERATIONS ===================================
//============================= ACESS      ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

