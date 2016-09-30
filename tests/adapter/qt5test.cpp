#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include <isis/adapter/qt5/common.hpp>
#include <QImage>
#include <tuple>

namespace isis
{
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( qimage_test )
{
	static const size_t xsize=512,ysize=512;
	data::ValueArray<float> data(xsize*ysize);
	for(int x=0;x<512;x++){
		float value=std::sin(x*M_PI*2/xsize);
		for(int y=0;y<512;y++)
		{
			data[x+y*ysize] = value;
		}
	}
	data::scaling_pair scaling=data.getScalingTo(data::ValueArray<uint8_t>::staticID());
	const data::ValueArrayBase::Converter &c = data.getConverterTo(data::ValueArray<uint8_t>::staticID());
	
	BOOST_REQUIRE_EQUAL(scaling.first->as<float>(), 127.5);
	BOOST_REQUIRE_EQUAL(scaling.second->as<float>(), 127.5);

	QImage img=qt5::makeQImage(data,512,scaling);
	
	for(int x=0;x<512;x++){
		float value=std::sin(x*M_PI*2/xsize)*scaling.first->as<float>()+scaling.second->as<float>();
		for(int y=0;y<512;y++)
		{
			BOOST_CHECK_EQUAL(qGray(img.pixel(x,y)),std::round(value));
		}
	}
}

BOOST_AUTO_TEST_CASE( qimage_complex_test )
{
	static const size_t xsize=512,ysize=512;
	data::ValueArray<std::complex<float>> data(xsize*ysize);
	for(int x=0;x<512;x++){
		std::complex<float> value = std::polar<float>(x+1,x);
		for(int y=0;y<512;y++)
		{
			data[x+y*ysize] = value;
		}
	}
	
	//getScalingTo wont work here, as there is no conversion from complex to uint8_t
	const std::pair<util::ValueReference,util::ValueReference> minmax = data.getMinMax();
	const std::complex<float> min=minmax.first->as<std::complex<float>>(),max=minmax.second->as<std::complex<float>>();
	
	const float magnitude_min = std::abs(min),magnitude_max = std::abs(max);
	const float phase_min = std::arg(min),phase_max = std::arg(max);
	
	const data::ValueArrayBase::Converter &c = data::ValueArrayBase::getConverterFromTo(data::ValueArray<float>::staticID(),data::ValueArray<uint8_t>::staticID());
	
//	BOOST_REQUIRE_EQUAL(scaling.first->as<float>(), 127.5);
// 	BOOST_REQUIRE_EQUAL(scaling.second->as<float>(), 127.5);
// 
// 	QImage img=qt5::makeQImage(data,512,scaling);
// 	
// 	for(int x=0;x<512;x++){
// 		float value=std::sin(x*M_PI*2/xsize)*scaling.first->as<float>()+scaling.second->as<float>();
// 		for(int y=0;y<512;y++)
// 		{
// 			BOOST_CHECK_EQUAL(qGray(img.pixel(x,y)),std::round(value));
// 		}
// 	}
}

}
}

