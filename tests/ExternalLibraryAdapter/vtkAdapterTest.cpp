#define BOOST_TEST_MODULE VTKAdapterTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>

#include "CoreUtils/log.hpp"
#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "ExternalLibraryAdapter/vtkAdapter.hpp"

namespace isis{namespace test{
  
	BOOST_AUTO_TEST_CASE (VTKAdapterTest)
	{

		ENABLE_LOG(CoreLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(isis::ImageIoDebug,isis::util::DefaultMsgPrint,isis::info);
		ENABLE_LOG(isis::ImageIoLog,isis::util::DefaultMsgPrint,isis::info);

		//load an image and store it into the vtkAdapter
		isis::data::ImageList imgList = isis::data::IOFactory::load("/scr/feige1/tmp/data.nii", "");
		//forward the first image of the list as a single image
		isis::VTKAdapter vtkimg(imgList.front());
		//forward the whole imagelist
		isis::VTKAdapter vtkimgList(imgList);
			
	}
}}//end namespace 
		
		
		