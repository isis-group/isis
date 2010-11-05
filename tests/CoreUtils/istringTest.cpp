#define BOOST_TEST_MODULE TypeTest
#define NOMINMAX 1
#include <boost/test/included/unit_test.hpp>
#include <CoreUtils/istring.hpp>

namespace isis
{
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( istring_test )
{
	BOOST_CHECK_EQUAL(util::istring("HaLLo there"),util::istring("Hallo thERe"));
	BOOST_CHECK(util::istring("HaLLo there1")!=util::istring("Hallo thERe"));
	BOOST_CHECK_EQUAL(util::istring("Hallo thERe").find("there"),6);
}

}
}
