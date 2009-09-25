/*
 * ImageFormatNii.cpp
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 */

#include "ImageFormatNii.h"  // class implemented


/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ====================================

ImageFormatNii::ImageFormatNii() {
	printf("Konstruktor Nii\n");
}

ImageFormatNii::ImageFormatNii(
    const ImageFormatNii& ) {
}// ImageFormatNii

ImageFormatNii::~ImageFormatNii() {
}// ~ImageFormatNii


//============================= OPERATORS ====================================



//============================= OPERATIONS ===================================

std::string ImageFormatNii::suffixes(){
return std::string();
}
std::string ImageFormatNii::dialects(){
return std::string("dialekt1 dialekt2");
}
std::string ImageFormatNii::name(){
return "Nifti";
}

isis::data::Chunks ImageFormatNii::load ( std::string filename, std::string dialect ){
	return isis::data::Chunks();
}

bool ImageFormatNii::save ( const isis::data::Chunks& chunks, std::string filename, std::string dialect ){
	return false;
}
//============================= ACESS      ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

