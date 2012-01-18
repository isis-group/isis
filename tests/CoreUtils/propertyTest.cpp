/*
 * propertyTest.cpp
 *
 *  Created on: Sep 23, 2009
 *      Author: proeger
 */

// The BOOST_TEST_MODULE is similar to a unit test suite.
#define BOOST_TEST_MODULE PropertyTests
#include <boost/test/unit_test.hpp>
#include <CoreUtils/property.hpp>
#include <string>
#include <iostream>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE( property_init_test )
{
	/*  ENABLE_LOG(CoreLog,util::DefaultMsgPrint,info);
	    ENABLE_LOG(CoreDebug,util::DefaultMsgPrint,verbose_info);*/
	//  default constructor
	util::PropertyValue propZero;
	//  initializer
	util::PropertyValue propA = std::string( "Property01" );
	BOOST_CHECK_EQUAL( propA->toString(), "Property01" );
	//  default: not needed
	BOOST_CHECK( !propA.needed() );
	BOOST_CHECK( !propZero.needed() );
}

BOOST_AUTO_TEST_CASE( property_copy_test )
{
	// Test copy operator
	util::PropertyValue propA = ( int32_t )5;
	util::PropertyValue propB = propA;
	BOOST_CHECK_EQUAL( propB, ( int32_t )5 );
	//check for deep copy (change of propA shall not change propB)
	propA = ( int32_t )6;
	BOOST_CHECK_EQUAL( propA, ( int32_t )6 );
	BOOST_CHECK_EQUAL( propB, ( int32_t )5 );
}

BOOST_AUTO_TEST_CASE( property_compare_test )
{
	// trivial case - direct compare
	BOOST_CHECK_EQUAL( util::PropertyValue( 5 ), 5 );
	// using conversion - should be true, because 5.4 will be converted to int (and thus rounded to 5)
	BOOST_CHECK_EQUAL( util::PropertyValue( 5.4 ), 5 );
	// using conversion - should be false, because 5.5 will be converted to int (and thus rounded to 6)
	BOOST_CHECK( !( util::PropertyValue( 5.5 ) == 5 ) );
}

}
}
