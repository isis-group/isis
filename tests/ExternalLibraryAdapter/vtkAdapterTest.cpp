#define BOOST_TEST_MODULE VTKAdapterTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>

#include "CoreUtils/log.hpp"
#include "DataStorage/image.hpp"
#include "DataStorage/io_factory.hpp"
#include "ExternalLibraryAdapter/vtkAdapter.hpp"

#include <vtkImageViewer.h>
#include <vtkImageData.h>
#include <vtkImageWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>

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
		vtkImageViewer* viewer = vtkImageViewer::New();
		
		//load an image and store it into the vtkAdapter
// 		data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
		data::ImageList imgList = isis::data::IOFactory::load("/home/raid/tuerke/workspace/data.nii", "");
		BOOST_CHECK(not imgList.empty());
		vtkImageData* vtkImage = adapter::vtkAdapter::makeVtkImageObject(imgList.front());
		BOOST_CHECK(not vtkImage);
		
		//finally show one axial slice of the vtkImage, z=zdimension/2
		LOG(DataDebug,info) << "Showing vtkImageData object. z=zdimension/2 = " << vtkImage->GetDimensions()[2] / 2;
		viewer->SetZSlice(vtkImage->GetDimensions()[2] / 2);
		viewer->SetInput(vtkImage);
		viewer->Render();
		sleep(3);
	}
	
	
}}//end namespace 
		
		
		