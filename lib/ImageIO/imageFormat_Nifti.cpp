/*
 * ImageFormatNii.cpp
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 */

#include "imageFormat_Nifti.hpp"
namespace isis{ namespace image_io{
	
/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= OPERATIONS ===================================


std::string ImageFormat_Nifti::suffixes(){
	return std::string(".nii.gz .nii");
}
std::string ImageFormat_Nifti::dialects(){
	return std::string("fsl");
}
std::string ImageFormat_Nifti::name(){
	return "Nifti";
}

data::ChunkList ImageFormat_Nifti::load ( std::string filename, std::string dialect ){
	return data::ChunkList();
}

bool ImageFormat_Nifti::write(const data::Image &image,std::string filename,std::string dialect){
	return false;
}

//============================= ACCESS     ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

}}