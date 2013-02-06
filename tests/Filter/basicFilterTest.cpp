#define BOOST_TEST_MODULE FilterTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>

#include "BasicFilter/GaussianFilter.hpp"
#include "BasicFilter/FrequencyFilter.hpp"
#include <DataStorage/io_factory.hpp>
#include "filter.hpp"

namespace isis
{
namespace test
{

data::Image createEmptyImage( util::ivector4 size )
{
	data::MemChunk<uint8_t> myChunk ( size[0], size[1], size[2], size[3] ) ;
	myChunk.setPropertyAs<util::fvector3>( "indexOrigin", util::fvector3() );
	myChunk.setPropertyAs<util::fvector3>( "rowVec", util::fvector3( 1, 0, 0 ) );
	myChunk.setPropertyAs<util::fvector3>( "columnVec", util::fvector3( 0, 1, 0 ) );
	myChunk.setPropertyAs<util::fvector3>( "voxelSize", util::fvector3( 1, 1, 1 ) );
	myChunk.setPropertyAs<uint16_t>( "acquisitionNumber", 0 );
	return data::Image( myChunk );
}

class TestFilter1 : public filter::_internal::ImageFilterInPlace
{
public:
	std::string getFilterName() const { return std::string( "TestFilter1" ); }

	bool isValid() const {
		return !parameters[ "value_to_add" ].isEmpty();
	}

	bool process( data::Image &image ) {
		const int32_t vta = parameters["value_to_add"].as<int32_t>();
		data::TypedImage<uint16_t> tImage ( image );

		for( data::TypedImage<uint16_t>::iterator iIter = tImage.begin(); iIter != tImage.end(); iIter++ ) {
			*iIter = vta;
		}

		image = tImage;
		return true;
	}
};

class CheckFilter : public filter::_internal::ImageFilterInPlace
{
	std::string getFilterName() const { return std::string( "CheckFilter" ); }

	bool isValid() const {
		return !parameters[ "value_to_check" ].isEmpty();
	}
	bool process( data::Image &image ) {
		bool isCorrect = true;
		const int32_t value_to_check = parameters["value_to_check"].as<int32_t>();
		data::TypedImage<uint16_t> tImage ( image );

		for( data::TypedImage<uint16_t>::iterator iIter = tImage.begin(); iIter != tImage.end(); iIter++ ) {
			if( *iIter != value_to_check ) {
				isCorrect = false;
			}
		}

		results["isCorrect"] = isCorrect;
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
	myTestFilter1.setParameter( "value_to_add", 10 );
	BOOST_CHECK( myTestFilter1.isValid() );
	myTestFilter1.run( myImage );
	myCheckFilter.setParameter( "value_to_check", 10 );
	myCheckFilter.run( myImage );
	BOOST_CHECK_EQUAL( myCheckFilter.getResults()["isCorrect"], true );

}

BOOST_AUTO_TEST_CASE( gaussian_filter_test )
{
	data::Image myImage = createEmptyImage( util::ivector4( 100, 100, 100, 1 ) );
	filter::GaussianFilter myGaussianFilter;
	myGaussianFilter.setParameter( "sigma", 1.5 );
	BOOST_CHECK( myGaussianFilter.run( myImage ) );
}

BOOST_AUTO_TEST_CASE( frequency_filter_test )
{
	data::Image myImage = data::IOFactory::load( "/SCR/DATA/gaby/ldopa/raw/ldopa_s01.v" ).front();
	//  data::Image myImage = createEmptyImage( util::ivector4(100,100,100,100) );
	//  myImage.setPropertyAs<uint16_t>("repetitionTime", 2000);
	filter::FrequencyFilter myFrequencyFilter;
	myFrequencyFilter.setParameter( "dimension", 3 );
	myFrequencyFilter.run( myImage );
	//  BOOST_CHECK( myFrequencyFilter.run( myImage ) );
	data::IOFactory::write( myImage, "/tmp/muddi.nii" );
}

}
}
