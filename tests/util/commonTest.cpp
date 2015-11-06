#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1

#include <boost/test/unit_test.hpp>
#include "util/common.hpp"

namespace isis
{
namespace test
{

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( fuzzy_equal_test )
{
	float a0 = 1;
	float b0 = 1 + std::numeric_limits< float >::epsilon();
	float a1 = 1 - std::numeric_limits< float >::epsilon();
	float b1 = 1;
	float a2 = 1;
	float b2 = 1 + std::numeric_limits< float >::epsilon() * 2;
	double a3 = 1;
	double b3 = 1 + std::numeric_limits< float >::epsilon();
	double a4 = 1;
	double b4 = 1 + std::numeric_limits< double >::epsilon();

	BOOST_CHECK( util::fuzzyEqual( a0, b0, 1 ) ); // distance of epsilon is considered equal with scaling of "1"
	BOOST_CHECK( util::fuzzyEqual( a1, b1, 1 ) ); // distance of epsilon is considered equal with scaling of "1"
	BOOST_CHECK( !util::fuzzyEqual( a2, b2, 1 ) ); // distance of epsilon*2 is _not_ considered equal anymore (with scaling of "1")
	BOOST_CHECK( util::fuzzyEqual( a2, b2, 2 ) ); // but with scaling of "2" it is
	BOOST_CHECK( !util::fuzzyEqual( a3, b3, 1 ) ); // distance of epsilon for float is _not_ considered equal for double
	BOOST_CHECK( util::fuzzyEqual( a4, b4, 1 ) ); // distance of epsilon for float is _not_ considered equal for double
}

BOOST_AUTO_TEST_CASE( string_to_list_test )
{
	const std::list<int> i_numbers{1,2,3,4,50};
	const std::list<std::string> s_numbers{"1","2","3","4","50"};
	
	BOOST_CHECK_EQUAL(util::stringToList<std::string>("1,2;3\t4 50"),s_numbers);
	BOOST_CHECK_EQUAL(util::stringToList<int>("1,2;3\t4  50"),i_numbers);
	
	const std::list<std::string> multiline{"Hello","there","world  (how are you)"};
	BOOST_CHECK_EQUAL(util::stringToList<std::string>("Hello\rthere\nworld  (how are you)",std::regex("[[.newline.][.carriage-return.]]")),multiline);
}

}
}
