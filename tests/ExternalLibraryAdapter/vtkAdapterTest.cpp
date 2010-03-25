#define BOOST_TEST_MODULE VTKAdapterTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include <list>

#include "CoreUtils/log.hpp"
#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "ExternalLibraryAdapter/vtkAdapter.hpp"

#include <vtkImageImport.h>
#include <vtkImageViewer.h>
#include <vtkImageData.h>

namespace isis{namespace test{
  
	BOOST_AUTO_TEST_CASE (VTKAdapterTest)
	{

		ENABLE_LOG(CoreLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(data::Runtime,util::DefaultMsgPrint,error);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,info);
		//ENABLE_LOG(isis::ImageIoDebug,isis::util::DefaultMsgPrint,isis::info);
		//ENABLE_LOG(isis::ImageIoLog,isis::util::DefaultMsgPrint,isis::info);
		
		//load an image and store it into the vtkAdapter
		data::ImageList imgList = isis::data::IOFactory::load("/SCR/rhenium/Images/nifti/S5_FLASH_3D_3Echoes_1.nii", "");
		BOOST_CHECK(not imgList.empty());
		
		std::list<vtkImageData*> vtkList = adapter::VTKAdapter::makeVtkImageList(imgList.front());
		vtkImageViewer* viewer = vtkImageViewer::New();
		viewer->SetInput(vtkList.front());
		viewer->SetColorWindow(20.0);
		viewer->SetColorLevel(0.0);
		viewer->Render();
		sleep(10);
		
		
	}
}}//end namespace 
		
		
		