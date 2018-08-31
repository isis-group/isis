#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include "../../isis/adapter/itk4/itkAdapter.hpp"
#include "../../isis/adapter/itk4/common.hpp"
#include "../../isis/core/chunk.hpp"
#include "../../isis/core/io_factory.hpp"

#include <itkImage.h>
#include <itkImageFileWriter.h>


namespace isis
{
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( itkimage_test )
{
	typedef itk::Image<float,4> ImageType;
	typedef itk::ImageFileWriter<ImageType> WriterType;
	
	itk4::enableLog<util::DefaultMsgPrint>( info );
	
	auto images=data::IOFactory::load("delme.null");
	
	itk4::itkAdapter adapter;
	ImageType::Pointer img=adapter.makeItkImageObject<ImageType>( images.front() );
	
	ImageType::DirectionType dir;
	
	WriterType::Pointer outputWriter = WriterType::New();
	outputWriter->SetFileName("/tmp/itk.nii");
	outputWriter->SetInput(img);
	outputWriter->Update();
	
	data::IOFactory::write(adapter.makeIsisImageObject<ImageType>(img),"/tmp/isis.nii");
}


}
}


