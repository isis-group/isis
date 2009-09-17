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

#include "DataStorage/ImageFormatFactory.h"
#include "DataStorage/ImageFormat.h"


int main() {
  using namespace boost::extensions;

  ImageFormat *pFormat;

    pFormat = ImageFormatFactory::GetInstance().CreateImageFormat("Nii");
    if (NULL == pFormat){
    	std::cerr << "Mist, bloeder. ImageFormat nicht da. " << std::endl;
    }

    std::cerr << "ich lese: " << pFormat->ImageFormatExtensions() << std::endl;




}
