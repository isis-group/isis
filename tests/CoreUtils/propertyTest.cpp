/*
 * propertyTest.cpp
 *
 *  Created on: Sep 23, 2009
 *      Author: proeger
 */

// The BOOST_TEST_MODULE is similar to a unit test suite.
#define BOOST_TEST_MODULE PropertyTests
#include <boost/test/included/unit_test.hpp>
#include <CoreUtils/property.hpp>
#include <string>
#include <iostream>

namespace isis{namespace test{
	
BOOST_AUTO_TEST_CASE(property_init_test)
{
	ENABLE_LOG(util::CoreLog,util::DefaultMsgPrint,util::info);
	ENABLE_LOG(util::CoreDebug,util::DefaultMsgPrint,util::verbose_info);
	
	//	default constructor
	util::PropertyValue propZero;

	//	initializer
	util::PropertyValue propA = std::string("Property01");
	BOOST_CHECK(propA->toString().compare("Property01") == 0);

	//	default: not needed
	BOOST_CHECK(!propA.needed());
	BOOST_CHECK(!propZero.needed());

}

BOOST_AUTO_TEST_CASE(property_copy_test)
{
	ENABLE_LOG(util::CoreLog,util::DefaultMsgPrint,util::info);
	ENABLE_LOG(util::CoreDebug,util::DefaultMsgPrint,util::verbose_info);

	// Test copy operator
	util::PropertyValue propA = 5;
	util::PropertyValue propB = propA;
	BOOST_CHECK(propB == 5);

	//check for deep copy (change of propA shall not change propB)
	propA = 6;
	BOOST_CHECK(propA == 6);
	BOOST_CHECK(propB == 5);
}

}}