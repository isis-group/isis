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
	
	const util::Value<float> magnitude_min = std::abs(min),magnitude_max = std::abs(max);
	const util::Value<float> phase_min = std::arg(min),phase_max = std::arg(max);
	
	const data::ValueArrayBase::Converter &c = data::ValueArrayBase::getConverterFromTo(data::ValueArray<float>::staticID(),data::ValueArray<uint8_t>::staticID());
	const data::scaling_pair magnitude_scale=c->getScaling(magnitude_min,magnitude_max,data::autoscale);
	const data::scaling_pair phase_scale=c->getScaling(phase_min,phase_max,data::autoscale);
	
	BOOST_REQUIRE_CLOSE(magnitude_scale.first->as<float>(), 0.5,1);
	BOOST_REQUIRE_CLOSE(magnitude_scale.second->as<float>(), -0.5,1);

	BOOST_CHECK_CLOSE(phase_scale.first->as<float>(), 255/(M_PI*2),1);
	BOOST_CHECK_CLOSE(phase_scale.second->as<float>(), 127.5,1);

	auto magnitude_transfer = [magnitude_scale](uchar *dst, const data::ValueArrayBase &line){
		const float scale=magnitude_scale.first->as<float>();
		const float offset=magnitude_scale.second->as<float>();
		for(const std::complex<float> &v:line.castToValueArray<std::complex<float>>()){
			*(dst++)=std::abs(v)*scale+offset;
		}
	};

	auto phase_transfer = [phase_scale](uchar *dst, const data::ValueArrayBase &line){
		const float scale=phase_scale.first->as<float>();
		const float offset=phase_scale.second->as<float>();
		for(const std::complex<float> &v:line.castToValueArray<std::complex<float>>()){
			*(dst++)=std::arg(v)*scale+offset;
		}
	};


	QImage magnitude_img=qt5::makeQImage(data,512,magnitude_transfer);
	QImage phase_img=qt5::makeQImage(data,512,phase_transfer);

	for(int x=0;x<512;x++){
		const std::complex<float> value = std::polar<float>(x+1,x);
		const uint8_t magnitude_value = std::abs(value)*magnitude_scale.first->as<float>()+magnitude_scale.second->as<float>();
		const uint8_t phase_value = std::arg(value)*phase_scale.first->as<float>()+phase_scale.second->as<float>();
		for(int y=0;y<512;y++)
		{
			BOOST_CHECK_EQUAL(qGray(magnitude_img.pixel(x,y)),magnitude_value);
			BOOST_CHECK_EQUAL(qGray(phase_img.pixel(x,y)),phase_value);
		}
	}
}

}
}

