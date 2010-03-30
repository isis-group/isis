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
		data::ImageList imgList = isis::data::IOFactory::load("/home/erik/workspace/data.nii", "");
		BOOST_CHECK(not imgList.empty());
		adapter::VTKAdapter::ImageVector vtkImageImageDataVector = adapter::VTKAdapter::makeVtkImageDataList(imgList.front());
		BOOST_CHECK(not vtkImageImageDataVector.empty());
		
		//finally show one axial slice of the vtkImage, z=zdimension/2
		LOG(DataDebug,info) << "Showing vtkImageData object. z=zdimension/2 = " << vtkImageImageDataVector.front()->GetDimensions()[2] / 2;
		viewer->SetZSlice(vtkImageImageDataVector.front()->GetDimensions()[2] / 2);
		viewer->SetInput(vtkImageImageDataVector.front());
		viewer->Render();
		sleep(3);
	}
	
	BOOST_AUTO_TEST_CASE (VTKAdapterTest4D)
	{

		ENABLE_LOG(CoreLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataLog,util::DefaultMsgPrint,warning);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,warning);
		ENABLE_LOG(data::Runtime,util::DefaultMsgPrint,error);
		ENABLE_LOG(DataDebug,util::DefaultMsgPrint,info);
		
		vtkImageViewer* viewer = vtkImageViewer::New();
		//load a 4d image
		data::ImageList imgList = isis::data::IOFactory::load("/home/erik/workspace/timeseries.nii", "");
// 		data::ImageList imgListTime = isis::data::IOFactory::load("test.null", "");
		BOOST_CHECK(not imgList.empty());
		adapter::VTKAdapter::ImageVector vtkImageImageDataVectorTime = adapter::VTKAdapter::makeVtkImageDataList(imgList.front());
		BOOST_CHECK(not vtkImageImageDataVectorTime.empty());
		LOG(DataDebug,info) << "Showing first of " << vtkImageImageDataVectorTime.size() << " objects. z=zdimension/2 = " << vtkImageImageDataVectorTime.front()->GetDimensions()[2] / 2;
		viewer->SetZSlice(vtkImageImageDataVectorTime.front()->GetDimensions()[2] / 2);
		viewer->SetInput(vtkImageImageDataVectorTime.front());
		viewer->Render();
		sleep(3);
	}
	
}}//end namespace 
		
		
		