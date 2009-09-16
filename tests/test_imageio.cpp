/*
 * main.cpp
 *
 *  Created on: Jul 30, 2009
 *      Author: hellrung
 */

#include <stdio.h>
#include <iostream>
#include <map>




#include <boost/function.hpp>
#include <boost/extension/factory.hpp>
#include <boost/extension/shared_library.hpp>
#include <boost/extension/type_map.hpp>
#include <boost/function.hpp>
#include <boost/scoped_ptr.hpp>

#include "../../LibraryToReadFromSO/animal.hpp"
//#include "../../LibraryToReadFromSO/LibTestClass.h"
#include "../../LibraryToReadFromSO/ImageFormatFactory.h"
#include "../../LibraryToReadFromSO/ImageFormat.h"


int main() {
  using namespace boost::extensions;

  ImageFormat *pFormat;
//  pFormat = ImageFormatFactory::GetInstance().CreateImageFormat("Nifti");
//      if ( true == pFormat->ImageFormatIsFunny() ){
//    	  std::cerr << "ich bin funny" << std::endl;
//      }
//      else{
//    	  std::cerr << "ich bin nicht funny" << std::endl;
//      }
//      std::cerr << "ich lese: " << pFormat->ImageFormatExtensions() << std::endl;
//
//  pFormat = ImageFormatFactory::GetInstance().CreateImageFormat("DCM");
//  if ( true == pFormat->ImageFormatIsFunny() ){
//	  std::cerr << "ich bin funny" << std::endl;
//  }
//  else{
//	  std::cerr << "ich bin nicht funny" << std::endl;
//  }
//  std::cerr << "ich lese: " << pFormat->ImageFormatExtensions() << std::endl;
//
//  if (NULL != pFormat){
//	  delete pFormat;
//	  pFormat = NULL;
//  }



    pFormat = ImageFormatFactory::GetInstance().CreateImageFormat("TestFormat");
        if ( true == pFormat->ImageFormatIsFunny() ){
      	  std::cerr << "ich bin funny" << std::endl;
        }
        else{
      	  std::cerr << "ich bin nicht funny" << std::endl;
        }
        std::cerr << "ich lese: " << pFormat->ImageFormatExtensions() << std::endl;

//
//  std::string library_path = "libLibraryToReadFromSO.so";
//
//
//
//  		shared_library lib(library_path);
//
//  		if(!lib.open()) {
//  			std::cerr << "Library failed to open: " << library_path << std::endl;
//  			return false;
//  		}
//  		std::cerr << "Library is open: " << library_path << "   " << lib.is_open() << std::endl;
//  		type_map types;
//  		if(!lib.call(types)) {
// 			std::cerr << "Function not found!" << std::endl;
//  			return false;
//  		}
//
//  		std::map<std::string, factory<animal, int> >& factories(types.get());
//  		if (factories.empty()) {
//  			std::cerr << "No animals found!" << std::endl;
//			return 1;
//  		}
//  for(std::map<std::string, factory<animal, int> >::iterator it = factories.begin(); it != factories.end(); ++it) {
//  			std::cout << "Creating animal factory: " << it->first << std::endl;
//  			it->second.create(5);
//  		}




}
