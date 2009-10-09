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

BOOST_AUTO_TEST_CASE(property_init_test)
{

	//	default constructor
	isis::util::PropertyValue propZero;

	//	initializer
	isis::util::PropertyValue propA = std::string("Property01");
	BOOST_CHECK(propA->toString().compare("Property01") == 0);

	//	copy constructor
	isis::util::PropertyValue propB = propA;
	BOOST_CHECK(propB->toString().compare("Property01") == 0);

	//	default: not needed
	BOOST_CHECK(!propA.needed());
	BOOST_CHECK(!propZero.needed());

}

BOOST_AUTO_TEST_CASE(property_copy_test)
{
	// Test copy operator
	isis::util::PropertyValue propA = 5;
	isis::util::PropertyValue propB = propA;
	BOOST_CHECK(propB == 5);

	propA = 6;
	BOOST_CHECK(propB == 5);
}