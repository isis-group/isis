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

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE ( ISIS_to_ITK )
{
	data::enable_log<util::DefaultMsgPrint>( error );
	//just to make sure the wanted file exists
	FILE *f = fopen( "test.null", "w" );
	fclose( f );
	typedef itk::Image<short, 4> MyImageType;
	itk::ImageFileWriter<MyImageType>::Pointer writer = itk::ImageFileWriter<MyImageType>::New();
	// data::ImageList imgList = isis::data::IOFactory::load("test.null", "");
	data::ImageList imgList = isis::data::IOFactory::load( "/scr/kastanie1/DATA/isis/data_fmrt.nii", "" );
	BOOST_CHECK( not imgList.empty() );
	MyImageType::Pointer itkImage = MyImageType::New();
	itkImage = adapter::itkAdapter::makeItkImageObject<MyImageType>( imgList.front() );
	writer->SetInput( itkImage );
	writer->SetFileName( "ISIS_to_ITK.nii" );
	writer->Update();
}


BOOST_AUTO_TEST_CASE ( ITK_to_ISIS )
{
	data::enable_log<util::DefaultMsgPrint>( error );
	typedef itk::Image<short, 4> MyImageType;
	itk::ImageFileReader<MyImageType>::Pointer reader = itk::ImageFileReader<MyImageType>::New();
	reader->SetFileName( "/scr/kastanie1/DATA/isis/data_fmrt.nii" );
	reader->Update();
	data::ImageList isisImageList;
	isisImageList = isis::adapter::itkAdapter::makeIsisImageObject<MyImageType>( reader->GetOutput() , false );
	data::IOFactory::write( isisImageList, "ITK_to_ISIS.nii", "" );
}

BOOST_AUTO_TEST_CASE ( ISIS_to_ITK_to_ISIS )
{
	data::enable_log<util::DefaultMsgPrint>( error );
	typedef itk::Image<short, 4> MyImageType;
	MyImageType::Pointer itkImage = MyImageType::New();
	data::ImageList isisImageList1 = isis::data::IOFactory::load( "/scr/kastanie1/DATA/isis/data_fmrt.nii", "" );
	itkImage = adapter::itkAdapter::makeItkImageObject<MyImageType>( isisImageList1.front() );
	data::ImageList isisImageList2 = isis::adapter::itkAdapter::makeIsisImageObject<MyImageType>( itkImage );
	data::IOFactory::write( isisImageList2, "ISIS_to_ITK_to_ISIS.nii", "" );
}

BOOST_AUTO_TEST_CASE ( ITK_to_ISIS_to_ITK )
{
	data::enable_log<util::DefaultMsgPrint>( error );
	typedef itk::Image<short, 4> MyImageType;
	MyImageType::Pointer itkImage = MyImageType::New();
	itk::ImageFileReader<MyImageType>::Pointer reader = itk::ImageFileReader<MyImageType>::New();
	itk::ImageFileWriter<MyImageType>::Pointer writer = itk::ImageFileWriter<MyImageType>::New();
	reader->SetFileName( "/scr/kastanie1/DATA/isis/data_fmrt.nii" );
	reader->Update();
	data::ImageList isisImageList;
	isisImageList = isis::adapter::itkAdapter::makeIsisImageObject<MyImageType>( reader->GetOutput() );
	itkImage = isis::adapter::itkAdapter::makeItkImageObject<MyImageType>( isisImageList.front() );
	writer->SetFileName( "ITK_to_ISIS_to_ITK.nii" );
	writer->SetInput( itkImage );
	writer->Update();
}

}
}//end namespace

