#define BOOST_TEST_MODULE VTKAdapterTest
#include <boost/test/included/unit_test.hpp>
#include <boost/foreach.hpp>
#include <list>

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
		FILE* f = fopen("test.null", "w");
		fclose(f);
		vtkImageViewer* viewer1 = vtkImageViewer::New();
		vtkImageViewer* viewer2 = vtkImageViewer::New();
		//load an image and store it into the vtkAdapter
// 		data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
		data::ImageList imgList = isis::data::IOFactory::load("/home/erik/workspace/data.nii", "");
		BOOST_CHECK(not imgList.empty());
		std::list< vtkSmartPointer<vtkImageData> > vtkImageImageDataList = adapter::VTKAdapter::makeVtkImageDataList(imgList.front());
		BOOST_CHECK(not vtkImageImageDataList.empty());
		//finally show one axial slice of the vtkImage, z=zdimension/2
		
		
		LOG(DataDebug,info) << "Showing vtkImageData object. z=zdimension/2 = " << vtkImageImageDataList.front()->GetDimensions()[2] / 2;
		viewer1->SetZSlice(vtkImageImageDataList.front()->GetDimensions()[2] / 2);
		viewer1->SetInput(vtkImageImageDataList.front());
		viewer1->Render();
		sleep(3);

		//load a 4d image
		data::ImageList imgListTime = isis::data::IOFactory::load("/home/erik/workspace/timeseries.nii", "");
// 		data::ImageList imgListTime = isis::data::IOFactory::load("test.null", "");
		BOOST_CHECK(not imgListTime.empty());
		std::list< vtkSmartPointer<vtkImageData> > vtkImageImageDataListTime = adapter::VTKAdapter::makeVtkImageDataList(imgListTime.front());
		BOOST_CHECK(not vtkImageImageDataListTime.empty());
		LOG(DataDebug,info) << "Showing first of " << vtkImageImageDataListTime.size() << " objects. z=zdimension/2 = " << vtkImageImageDataListTime.front()->GetDimensions()[2] / 2;
		viewer2->SetZSlice(vtkImageImageDataListTime.front()->GetDimensions()[2] / 2);
		viewer2->SetInput(vtkImageImageDataListTime.front());
		viewer2->Render();
		sleep(3);
				
	}
}}//end namespace 
		
		
		