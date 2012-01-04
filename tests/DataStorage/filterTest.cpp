#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <DataStorage/filter.hpp>


namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE( filter_test )
{
	class TestFilter : public isis::filter::ChunkFilter<2, 1>
	{
	public:
		virtual bool process() {
			std::cout <<  m_input[0]->getSizeAsVector() << std::endl;
			return true;
		}
	};

	data::MemChunk<uint16_t> ch1 ( 10, 10, 10 );
	data::MemChunk<uint16_t> ch2 ( 10, 10, 10 );

	std::list<data::Chunk> chList;
	chList.push_back( ch1 );

	TestFilter myTestProcess;
	myTestProcess.setInput( chList );
	BOOST_CHECK( !myTestProcess.isValid() );
	chList.push_back( ch2 );
	myTestProcess.setInput( chList );
	BOOST_CHECK( myTestProcess.isValid() );
	myTestProcess.process();


}

}
}