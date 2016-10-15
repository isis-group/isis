#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include "../../isis/adapter/qt5/common.hpp"
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
	const float min=minmax.first->as<float>(),max=minmax.second->as<float>();
	
	const data::ValueArrayBase::Converter &c = data::ValueArrayBase::getConverterFromTo(data::ValueArray<float>::staticID(),data::ValueArray<uint8_t>::staticID());
	const data::scaling_pair scaling=c->getScaling(*minmax.first,*minmax.second,data::autoscale);
	
	BOOST_REQUIRE_CLOSE(scaling.first->as<float>(), 0.5,1);
	BOOST_REQUIRE_CLOSE(scaling.second->as<float>(), -0.5,1);

	auto magnitude_transfer = [scaling](uchar *dst, const data::ValueArrayBase &line){
		const float scale=scaling.first->as<float>();
		const float offset=scaling.second->as<float>();
		for(const std::complex<float> &v:line.castToValueArray<std::complex<float>>()){
			*(dst++)=std::abs(v)*scale+offset;
		}
	};

	auto phase_transfer = [](uchar *dst, const data::ValueArrayBase &line){
		const float scale=M_PI/128;
		const float offset=128;
		for(const std::complex<float> &v:line.castToValueArray<std::complex<float>>()){
			*(dst++)=std::arg(v)*scale+offset;
		}
	};


	QImage magnitude_img=qt5::makeQImage(data,512,magnitude_transfer);
	QImage phase_img=qt5::makeQImage(data,512,phase_transfer);

	for(int x=0;x<512;x++){
		const std::complex<float> value = std::polar<float>(x+1,x);
		const uint8_t magnitude_value = std::abs(value)*scaling.first->as<float>()+scaling.second->as<float>();
		const uint8_t phase_value = std::arg(value)*M_PI/128 + 128;
		for(int y=0;y<512;y++)
		{
			BOOST_CHECK_EQUAL(qGray(magnitude_img.pixel(x,y)),magnitude_value);
			BOOST_CHECK_EQUAL(qGray(phase_img.pixel(x,y)),phase_value);
		}
	}
}

}
}

