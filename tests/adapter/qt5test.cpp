#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include <isis/adapter/qt5/common.hpp>
#include <QImage>

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

}
}

