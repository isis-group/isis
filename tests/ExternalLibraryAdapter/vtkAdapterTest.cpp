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
#include <vtkSmartPointer.h>

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
		std::list< vtkSmartPointer<vtkImageImport> > vtkImageImportList = adapter::VTKAdapter::makeVtkImageImportList(imgList.front());
		std::list< vtkSmartPointer<vtkImageData> > vtkImageImageDataList = adapter::VTKAdapter::makeVtkImageDataList(imgList.front());
		BOOST_CHECK(not vtkImageImportList.empty());
		BOOST_CHECK(not vtkImageImageDataList.empty());
		//finally show one axial slice of the vtkImage, z=100
		vtkImageViewer* viewer1 = vtkImageViewer::New();
		vtkImageViewer* viewer2 = vtkImageViewer::New();
		LOG(DataDebug,info) << "Showing vtkImageImport object";
		viewer1->SetZSlice(100);
		viewer1->SetInputConnection(vtkImageImportList.front()->GetOutputPort());
		viewer1->Render();
		sleep(3);
		LOG(DataDebug,info) << "Showing vtkImageData object";
		viewer2->SetZSlice(100);
		viewer2->SetInput(vtkImageImageDataList.front());
		viewer2->Render();
		sleep(3);
	}
}}//end namespace 
		
		
		