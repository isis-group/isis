/*
 * ImageFormatDCM.cpp
 *
 *  Created on: Sep 4, 2009
 *      Author: hellrung
 */

#include <map>
#include <iostream>

//for extension
#include <boost/extension/extension.hpp>
#include <boost/extension/factory.hpp>
#include <boost/extension/type_map.hpp>

//for reflection
#include <boost/reflection/reflection.hpp>

//Local includes
#include "ImageFormatDCM.h"  // class implemented
#include "DataStorage/ImageFormatFactory.h"


/////////////////////////////// PUBLIC ///////////////////////////////////////
/*
 * The unique Id of this format
 * */
const std::string ImageFormatDCM::FormatID = "DCM";

/*
 * register the type and remember status in a flag
 * */
const bool ImageFormatDCM::isRegistered = ImageFormatFactory::GetInstance().RegisterImageFormat(
	ImageFormatDCM::FormatID, ImageFormatDCM::CreateImageFormatDCM);


//============================= LIFECYCLE ====================================

ImageFormatDCM::ImageFormatDCM()
{
}// ImageFormatDCM

ImageFormatDCM::ImageFormatDCM(const ImageFormatDCM& from)
{
}// ImageFormatDCM

ImageFormatDCM::~ImageFormatDCM()
{
}// ~ImageFormatDCM


//============================= OPERATORS ====================================

ImageFormatDCM& 
ImageFormatDCM::operator=(const ImageFormatDCM& from)
{
	if (this == &from) {
		return *this;}


   return *this;

}// =

//============================= OPERATIONS ===================================
ImageFormat* ImageFormatDCM::CreateClone() const{
	return new ImageFormatDCM(*this);
}

bool ImageFormatDCM::ImageFormatIsFunny(){
	return false;
}

std::string ImageFormatDCM::ImageFormatExtensions(){
	return "dicom";
}

bool ImageFormatDCM::CanHandleThisFile(){
	return true;
}

//============================= ACESS      ===================================

BOOST_EXTENSION_TYPE_MAP_FUNCTION {
	using namespace boost::extensions;
	std::map<std::string, factory<ImageFormat> >& format_factories(types.get());
	format_factories["FormatDCM"].set<ImageFormatDCM> ();
}

using namespace boost::reflections;

extern "C"
void BOOST_EXTENSION_EXPORT_DECL
extension_export_format(std::map<std::string, reflection> reflection_map) {
	reflection_map["ImageFormat"].reflect<ImageFormatDCM>()
	.constructor()
	.function(&ImageFormatDCM::ImageFormatExtensions, "get_extensions");

}

//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

