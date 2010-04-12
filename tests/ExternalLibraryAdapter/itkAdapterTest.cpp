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
  
	BOOST_AUTO_TEST_CASE (ITKAdapterTest)
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
		typedef itk::Image<int, 4> MyImageType;
		itk::ImageFileWriter<MyImageType>::Pointer writer = itk::ImageFileWriter<MyImageType>::New();
		//load an image and store it into the vtkAdapter
// 		data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
		data::ImageList imgList = isis::data::IOFactory::load("/home/raid/tuerke/workspace/data_fmrt.nii", "");
		BOOST_CHECK(not imgList.empty());
		MyImageType::Pointer itkImage = MyImageType::New();
		itkImage = adapter::itkAdapter::makeItkImageObject<MyImageType>(imgList.front());
		writer->SetInput(itkImage);
		writer->SetFileName("itkAdapterTest_output.nii");
		writer->Update();				
	}
	
	
}}//end namespace 
		
		