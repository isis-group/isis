#define BOOST_TEST_MODULE itkAdapterTest
#include <boost/test/included/unit_test.hpp>

#include "CoreUtils/log.hpp"
#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "ExternalLibraryAdapter/itkAdapter.hpp"

#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkSmartPointer.h>

namespace isis{namespace test{
  
	BOOST_AUTO_TEST_CASE (VTKAdapterTest3D)
	{

		ENABLE_LOG(CoreLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(data::Runtime,util::DefaultMsgPrint,error);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,info);
		//ENABLE_LOG(isis::ImageIoDebug,isis::util::DefaultMsgPrint,isis::info);
		//ENABLE_LOG(isis::ImageIoLog,isis::util::DefaultMsgPrint,isis::info);
		// just to make sure the wanted file exists
		FILE* f = fopen("test.null", "w");
		fclose(f);		
		//load an image and store it into the vtkAdapter
		data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
		BOOST_CHECK(not imgList.empty());
		itk::Image<unsigned short, 3>::Pointer itkImage = itk::Image<unsigned short, 3>::New();
		itkImage = adapter::itkAdapter::makeItkImageObject<unsigned short, 3>(imgList.front());
				
	}
	
	
}}//end namespace 
		
		