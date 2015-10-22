#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1
#include <boost/test/unit_test.hpp>
#include <util/istring.hpp>

namespace isis
{
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( istring_test )
{
	BOOST_CHECK_EQUAL( util::istring( "HaLLo there" ), util::istring( "Hallo thERe" ) );
	BOOST_CHECK( util::istring( "HaLLo there1" ) != util::istring( "Hallo thERe" ) );
	BOOST_CHECK_EQUAL( util::istring( "Hallo thERe" ).find( "there" ), 6 );
	BOOST_CHECK_EQUAL( boost::lexical_cast<util::istring>( 1234 ), "1234" );
	BOOST_CHECK_EQUAL( boost::lexical_cast<int>( util::istring( "1234" ) ), 1234 );
	BOOST_CHECK_EQUAL( boost::lexical_cast<util::istring>( std::string( "Test" ) ), "Test" );
	BOOST_CHECK_EQUAL( boost::lexical_cast<std::string>( util::istring( "Test" ) ), "Test" );
}

}
}
