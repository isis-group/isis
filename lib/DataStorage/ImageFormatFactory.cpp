/*
 * ImageFormatFactory.cpp
 *
 *  Created on: Aug 11, 2009
 *      Author: hellrung
 */

#include "ImageFormatFactory.h"  // class implemented
#include <stdexcept>
#include <iostream>
#include <map>

#include <boost/extension/factory.hpp>
#include <boost/extension/shared_library.hpp>
#include <boost/extension/type_map.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>
//for reflection
#include <boost/reflection/reflection.hpp>
#include <boost/extension/convenience.hpp>
/////////////////////////////// PUBLIC ///////////////////////////////////////

//============================= LIFECYCLE ====================================

ImageFormatFactory::ImageFormatFactory() {
	LoadFromPluginPath();
}// ImageFormatFactory

ImageFormatFactory::ImageFormatFactory(
    const ImageFormatFactory&) {
}// ImageFormatFactory

ImageFormatFactory::~ImageFormatFactory() {
}// ~ImageFormatFactory


//============================= OPERATORS ====================================

ImageFormatFactory&
ImageFormatFactory::operator=(
    const ImageFormatFactory&) {//TODO
}// =

//============================= OPERATIONS ===================================

bool ImageFormatFactory::RegisterImageFormat(
    const std::string& strFormatId,  CreateImageFormatCallback fnCreateFn) {

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
bool ImageFormatFactory::LoadFromPluginPath(){
	using namespace boost::extensions;

	//TODO: Aufdroeseln in Pfad und Libname und while (libs_available) do...

	std::string arrayOfLibs[2];
	arrayOfLibs[0] = "../../ImageFormatTest/Release/libImageFormatTest.so";
	//arrayOfLibs[0] = "/SCR/Programming2/hellrung/ImageFormatTest/Release/libImageFormatTest.so";
	//std::string library_path = "../../ImageFormatTest/Debug/libImageFormatTest.so";
	for (int index = 0; index < 1; index++) {


		shared_library lib(arrayOfLibs[index]);
		std::cerr << "Library to open: " << arrayOfLibs[index] << std::endl;

		if(!lib.open()) {
			std::cerr << "Library failed to open: " << arrayOfLibs[index] << std::endl;
			return false;
		}
		std::cerr << "Library opened: " << arrayOfLibs[index] << std::endl;
		// get a map of ImageFormat types the lib is containing
		// defined in lib by BOOST_EXTENSION_TYPE_MAP_FUNCTION
		type_map types;
		if(!lib.call(types)) {
			std::cerr << "Function not found!" << std::endl;
			return false;
		}
		// write all types and a key into our own map
		// factory<ImageFormat> says type "ImageFormat" without parameter for constructor
		std::map<std::string, factory<ImageFormat> >& factories(types.get());
		if(factories.empty()) {
			std::cerr << "ImageFormat not found!" << std::endl;
			return false;
		}

		//only for test purposes
		for(std::map<std::string, factory<ImageFormat> >::iterator it = factories.begin(); it != factories.end(); ++it) {
			std::cout << "Creating a format using factory: " << it->first << std::endl;
		}

		//REFLECTIONS
		using boost::reflections::instance;
		using boost::reflections::instance_constructor;
		using boost::reflections::reflection;
		// Create a mapping of reflections to strings that
		// will be populated inside the shared library.
		std::map<std::string, reflection> reflection_map;
		// Call an exported function to populate
		// reflection_map
		lib.get<void, std::map<std::string, reflection> &> ("extension_export_format")(reflection_map);
		// Pull out a reflection named "ImageFormatTest"
		reflection & first_reflection = reflection_map["ImageFormat"];
		instance_constructor<int> first_constructor = first_reflection.get_constructor<int> ();
		instance first_instance = first_constructor(123);
		boost::reflections::function<std::string> GetExtensions = first_reflection.get_function<std::string> (
		    "get_extensions");
		std::cerr << "Extensions: " << GetExtensions(first_instance) << std::endl;

		lib.close();

	}
	return true;
}

//============================= ACESS      ===================================
//============================= INQUIRY    ===================================
/////////////////////////////// PROTECTED  ///////////////////////////////////

/////////////////////////////// PRIVATE    ///////////////////////////////////

