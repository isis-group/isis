#define BOOST_TEST_MODULE FilterTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>
#include <DataStorage/filter.hpp>


namespace isis
{
namespace test
{

data::Image createEmptyImage( util::ivector4 size ) {
	data::MemChunk<uint16_t> myChunk (size[0], size[1], size[2], size[3] ) ;
	myChunk.setPropertyAs<util::fvector4>("indexOrigin", util::fvector4() );
	myChunk.setPropertyAs<util::fvector4>("rowVec", util::fvector4(1,0,0,0) );
	myChunk.setPropertyAs<util::fvector4>("columnVec", util::fvector4(0,1,0,0) );
	myChunk.setPropertyAs<util::fvector4>("voxelSize", util::fvector4(1,1,1,1) );
	myChunk.setPropertyAs<uint16_t>("acquisitionNumber", 0 );
	return data::Image( myChunk );
}

class TestFilter1 : public filter::ImageFilter11
{
public:
	std::string getFilterName() const { return std::string("TestFilter1"); }

	bool isValid() const {
		return m_inputIsSet && m_parameterMap.hasProperty("value_to_add");
	}

	bool process() {
		const uint16_t vta = m_parameterMap.getPropertyAs<uint16_t>("value_to_add");
		for( data::Image::iterator iIter = m_input->begin(); iIter != m_input->end(); iIter++ ) {
			iIter->operator=( util::Value<uint16_t>(vta) );
		}
		m_output = m_input;
		return true;
	}
};

class CheckFilter : public filter::ImageFilter11
{
	std::string getFilterName() const { return std::string("CheckFilter"); }

	bool isValid() const {
		return m_inputIsSet && m_parameterMap.hasProperty("value_to_add");
	}

};

BOOST_AUTO_TEST_CASE( basic_filter_test )
{
	data::Image myImage = createEmptyImage(util::ivector4(10,10,10,10) );

	TestFilter1 myTestFilter1;
	BOOST_CHECK_EQUAL( myTestFilter1.getFilterName(), std::string("TestFilter1") );
	BOOST_CHECK( !myTestFilter1.isValid() );
	myTestFilter1.setParameter<uint16_t>( "value_to_add", 10 );
	BOOST_CHECK( !myTestFilter1.isValid() );
	myTestFilter1.setInput(myImage);
	BOOST_CHECK( myTestFilter1.isValid() );
	myTestFilter1.run();
	data::Image output = myTestFilter1.getOutput();

}



}
}
