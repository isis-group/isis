/*
 * ImageFormatFactory.cpp
 *
 *  Created on: Aug 11, 2009
 *      Author: hellrung
 */

#include "ImageFormatFactory.h"  // class implemented
#include <stdexcept>

/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ====================================

ImageFormatFactory::ImageFormatFactory() {
}// ImageFormatFactory

ImageFormatFactory::ImageFormatFactory(
    const ImageFormatFactory&) {
}// ImageFormatFactory

ImageFormatFactory::~ImageFormatFactory() {
}// ~ImageFormatFactory


//============================= OPERATORS ====================================



//============================= OPERATIONS ===================================

bool ImageFormatFactory::RegisterImageFormat(
    const std::string& strFormatId, const CreateImageFormatCallback& fnCreateFn) {
	return m_mapCreateFunctions.insert(CreateFnMap::value_type(strFormatId, fnCreateFn)).second;
	//		std::make_pair(strFormatId, fnCreateFn)).second; //alternatively, should do the same
}

bool ImageFormatFactory::UnregisterImageFormat(
    const std::string& strFormatId) {
	return (1 == m_mapCreateFunctions.erase(strFormatId)) ? true : false;
}

ImageFormat* ImageFormatFactory::CreateImageFormat(
	const std::string& strFormatId){
	CreateFnMap::const_iterator it = m_mapCreateFunctions.find(strFormatId);
	if(it == m_mapCreateFunctions.end()){
		//dunno what to do with this Id
		throw std::runtime_error("Unknown Id for image format"); //TODO: log
		//LOG
	}//I see, here your pointer to the wanted image format created by the "CreateImageFormatCallback" function
	return (it->second)();

}

//============================= ACESS      ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

