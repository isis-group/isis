#define BOOST_TEST_MODULE itkAdapterTest
#include <boost/test/included/unit_test.hpp>

#include "CoreUtils/log.hpp"
#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "ExternalLibraryAdapter/itkAdapter.hpp"

#include <itkImage.h>
#include <itkImageFileWriter.h>
#include <itkImageFileReader.h>
#include <itkSmartPointer.h>

namespace isis{namespace test{
  
	BOOST_AUTO_TEST_CASE (ITKAdapterTest)
	{
		data::enable_log<util::DefaultMsgPrint>(error);
		// just to make sure the wanted file exists
		FILE* f = fopen("test.null", "w");
		fclose(f);
		typedef itk::Image<short, 3> MyImageType;
		itk::ImageFileWriter<MyImageType>::Pointer writer = itk::ImageFileWriter<MyImageType>::New();
// 		data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
		data::ImageList imgList = isis::data::IOFactory::load("/scr/kastanie1/DATA/isis/data.nii", "");	
		BOOST_CHECK(not imgList.empty());
		MyImageType::Pointer itkImage = MyImageType::New();
		MyImageType::IndexType index;
		itkImage = adapter::itkAdapter::makeItkImageObject<MyImageType>(imgList.front());
		writer->SetInput(itkImage);
		writer->SetFileName("itkAdapterTest_output.nii");
		writer->Update();		
	}
	
	
}}//end namespace 
		
		