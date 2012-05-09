#define BOOST_TEST_MODULE FilterTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>
#include <DataStorage/filter.hpp>

#include "BasicFilter/GaussianFilter.hpp"
#include <DataStorage/io_factory.hpp>

namespace isis
{
namespace test
{

data::Image createEmptyImage( util::ivector4 size )
{
	data::MemChunk<uint8_t> myChunk ( size[0], size[1], size[2], size[3] ) ;
	myChunk.setPropertyAs<util::fvector4>( "indexOrigin", util::fvector4() );
	myChunk.setPropertyAs<util::fvector4>( "rowVec", util::fvector4( 1, 0, 0, 0 ) );
	myChunk.setPropertyAs<util::fvector4>( "columnVec", util::fvector4( 0, 1, 0, 0 ) );
	myChunk.setPropertyAs<util::fvector4>( "voxelSize", util::fvector4( 1, 1, 1, 1 ) );
	myChunk.setPropertyAs<uint16_t>( "acquisitionNumber", 0 );
	return data::Image( myChunk );
}

class TestFilter1 : public filter::ImageFilterInPlace
{
public:
	std::string getFilterName() const { return std::string( "TestFilter1" ); }

	bool isValid() const {
		return parameterMap.hasProperty( "value_to_add" );
	}

	bool process( data::Image &image ) {
		const uint16_t vta = parameterMap.getPropertyAs<uint16_t>( "value_to_add" );
		data::TypedImage<uint16_t> tImage ( image );

		for( data::TypedImage<uint16_t>::iterator iIter = tImage.begin(); iIter != tImage.end(); iIter++ ) {
			*iIter = vta;
		}

		image = tImage;
		return true;
	}
};

class CheckFilter : public filter::ImageFilterInPlace
{
	std::string getFilterName() const { return std::string( "CheckFilter" ); }

	bool isValid() const {
		return parameterMap.hasProperty( "value_to_check" );
	}
	bool process( data::Image &image ) {
		bool isCorrect = true;
		const uint16_t value_to_check = parameterMap.getPropertyAs<uint16_t>( "value_to_check" );
		data::TypedImage<uint16_t> tImage ( image );

		for( data::TypedImage<uint16_t>::iterator iIter = tImage.begin(); iIter != tImage.end(); iIter++ ) {
			if( *iIter != value_to_check ) {
				isCorrect = false;
			}
		}

		resultMap.setPropertyAs<bool>( "isCorrect", isCorrect );
		return true;
	}
};

BOOST_AUTO_TEST_CASE( basic_filter_test )
{
	data::Image myImage = createEmptyImage( util::ivector4( 100, 10, 10, 10 ) );
	TestFilter1 myTestFilter1;
	CheckFilter myCheckFilter;
	BOOST_CHECK_EQUAL( myTestFilter1.getFilterName(), std::string( "TestFilter1" ) );
	BOOST_CHECK( !myTestFilter1.isValid() );
	myTestFilter1.setParameter<uint16_t>( "value_to_add", 10 );
	BOOST_CHECK( myTestFilter1.isValid() );
	myTestFilter1.run( myImage );
	myCheckFilter.setParameter<uint16_t>( "value_to_check", 10 );
	myCheckFilter.run( myImage );
	BOOST_CHECK_EQUAL( myCheckFilter.getResult<bool>( "isCorrect" ), true );

}

BOOST_AUTO_TEST_CASE( gaussian_filter_test )
{
	data::Image myImage = createEmptyImage( util::ivector4( 200, 200, 200, 1 ) );
	filter::GaussianFilter myGaussianFilter;
	myGaussianFilter.setParameter<float>( "sigma", 1.5 );
	myGaussianFilter.run( myImage );
}

}
}
