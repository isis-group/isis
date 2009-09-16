/*
 * ImageFormatNii.cpp
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 */

#include "ImageFormatNii.h"  // class implemented
#include "DataStorage/ImageFormatFactory.h"
/////////////////////////////// PUBLIC ///////////////////////////////////////
/*
 * The unique Id of this format
 * */
const std::string ImageFormatNii::FormatID = "Nifti";

/*
 * register the type and remember status in a flag
 * */
const bool ImageFormatNii::isRegistered = ImageFormatFactory::GetInstance().RegisterImageFormat(
    ImageFormatNii::FormatID, ImageFormatNii::CreateImageFormatNii);
//============================= LIFECYCLE ====================================

ImageFormatNii::ImageFormatNii() {
	printf("Konstruktor Nii\n");
}

ImageFormatNii::ImageFormatNii(
    const ImageFormatNii& from) {
}// ImageFormatNii

ImageFormatNii::~ImageFormatNii() {
}// ~ImageFormatNii


//============================= OPERATORS ====================================

ImageFormatNii&
ImageFormatNii::operator=(
    const ImageFormatNii& from) {
	if(this == &from) {
		return *this;
	}

	return *this;

}// =

//============================= OPERATIONS ===================================
ImageFormat* ImageFormatNii::CreateClone() const{
	return new ImageFormatNii(*this);
}

bool ImageFormatNii::ImageFormatIsFunny(){
	return true;
}

std::string ImageFormatNii::ImageFormatExtensions(){
	return "nii";
}

bool ImageFormatNii::CanHandleThisFile(){
	return true;
}
//============================= ACESS      ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

