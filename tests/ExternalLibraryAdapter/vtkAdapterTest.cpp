#define BOOST_TEST_MODULE VTKAdapterTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "util/log.hpp"
#include "data/image.hpp"
#include "data/io_factory.hpp"
#include "ExternalLibraryAdapter/vtkAdapter.hpp"

#include <vtkImageViewer.h>
#include <vtkImageData.h>
#include <vtkImageWriter.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE ( VTKAdapterTest3D )
{
	data::enable_log<util::DefaultMsgPrint>( error );
	// just to make sure the wanted file exists
	FILE *f = fopen( "test.null", "w" );
	fclose( f );
	vtkImageViewer *viewer = vtkImageViewer::New();
	//load an image and store it into the vtkAdapter
	//      data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
	data::ImageList imgList = isis::data::IOFactory::load( "/scr/kastanie1/core/isis/data.nii", "" );
	BOOST_CHECK( not imgList.isEmpty() );
	vtkImageData *vtkImage = adapter::vtkAdapter::makeVtkImageObject( imgList.front() );
	BOOST_CHECK( not vtkImage );
	//finally show one axial slice of the vtkImage, z=zdimension/2
	LOG( DataDebug, info ) << "Showing vtkImageData object. z=zdimension/2 = " << vtkImage->GetDimensions()[2] / 2;
	viewer->SetZSlice( vtkImage->GetDimensions()[2] / 2 );
	viewer->SetInput( vtkImage );
	viewer->Render();
	sleep( 3 );
}


}
}//end namespace


