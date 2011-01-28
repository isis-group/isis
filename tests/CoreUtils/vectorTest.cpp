/*
* typeTest.cpp
*
* A unit test suite to confirm the capabilities of the
* utils::Type class.
*
*  Created on: Oct 19, 2009
*      Author: reimer
*/

#define BOOST_TEST_MODULE VectorTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "CoreUtils/vector.hpp"
#include <string.h>

namespace isis
{
namespace test
{

using util::fvector4;

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( vector_init_test )
{
	fvector4 test;
	float *start = &test[0];
	const float compare1[] = {0, 0, 0, 0};
	const float compare2[] = {42, 42, 42, 42};
	BOOST_CHECK( memcmp( start, compare1, sizeof( float ) * 4 ) == 0 );
	test.fill( 42 );
	BOOST_CHECK( memcmp( start, compare2, sizeof( float ) * 4 ) == 0 );
}

BOOST_AUTO_TEST_CASE( vector_output_test )
{
	fvector4 test1, test2;
	test2.fill( 42 );
	std::ostringstream o;
	o << test1;
	BOOST_CHECK_EQUAL( o.str(), "<0|0|0|0>" );
	o.str( "" );
	o << test2;
	BOOST_CHECK_EQUAL( o.str(), "<42|42|42|42>" );
	o.str( "" );
}

BOOST_AUTO_TEST_CASE( vector_op_test )
{
	fvector4 test1, test2;
	BOOST_CHECK_EQUAL( test1, test2 );
	test2.fill( 42 );
	BOOST_CHECK( test1 != test2 );
	BOOST_CHECK_EQUAL( ( test1 + test2 ), test2 ); //0+42 == 42
	BOOST_CHECK_EQUAL( test1 - test2, -test2 ); //0-42= -42
	BOOST_CHECK_EQUAL( test2 - test1, test2 ); // 42-0 = 42
	BOOST_CHECK_EQUAL( test2 + test2, test2 * 2 ); // 42+42 = 42*2
}
}
}