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
#include <vtkImageWriter.h>
#include <vtkPolyDataWriter.h>

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
		// just to make sure the wanted file exists
// 		FILE* f = fopen("test.null", "w");
// 		fclose(f);
		//load an image and store it into the vtkAdapter
// 		data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
		data::ImageList imgList = isis::data::IOFactory::load("/home/erik/workspace/data.nii", "");
		
		BOOST_CHECK(not imgList.empty());
		std::list<vtkImageImport*> vtkImageImportList = adapter::VTKAdapter::makeVtkImageImportList(imgList.front());
		BOOST_CHECK(not vtkImageImportList.empty());
		//finally show one axial slice of the vtkImage, z=100
		vtkImageViewer* viewer = vtkImageViewer::New();
		viewer->SetZSlice(100);
		viewer->SetInputConnection(vtkImageImportList.front()->GetOutputPort());
		viewer->Render();
		sleep(5);		
	}
}}//end namespace 
		
		
		