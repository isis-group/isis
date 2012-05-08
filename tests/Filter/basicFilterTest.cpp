#define BOOST_TEST_MODULE FilterTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>
#include <DataStorage/filter.hpp>


namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE( basic_filter_test )
{
	class TestFilter1 : public filter::ImageFilter11
	{
	public:
		std::string getFilterName() const { return std::string("TestFilter1"); }

		bool isValid() const {
			return true;
		}

		bool process() const {
		}
	};

	TestFilter1 myTestFilter1;
	BOOST_CHECK_EQUAL( myTestFilter1.getFilterName(), std::string("TestFilter1") );

}

}
}
