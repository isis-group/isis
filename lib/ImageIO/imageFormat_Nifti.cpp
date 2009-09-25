/*
 * ImageFormatNii.cpp
 *
 *  Created on: Aug 12, 2009
 *      Author: hellrung
 */

#include "imageFormat_Nifti.hpp"

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= OPERATIONS ===================================

std::string isis::image_io::ImageFormat_Nifti::suffixes(){
	return std::string("nii");
}
std::string isis::image_io::ImageFormat_Nifti::dialects(){
	return std::string("fsl");
}
std::string isis::image_io::ImageFormat_Nifti::name(){
	return "Nifti";
}

isis::data::ChunkList isis::image_io::ImageFormat_Nifti::load ( std::string filename, std::string dialect ){
	return isis::data::ChunkList();
}

bool isis::image_io::ImageFormat_Nifti::save ( const isis::data::ChunkList& chunks, std::string filename, std::string dialect ){
	return false;
}
//============================= ACCESS     ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

