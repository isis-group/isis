/*
 * typeTest.cpp
 *
 * A unit test suite to confirm the capabilities of the
 * isis::utils::Type class.
 *
 *  Created on: Sep 24, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE TypeTest
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <CoreUtils/type.hpp>

using isis::util::Type;

// TestCase object instantiation
BOOST_AUTO_TEST_CASE(type_init_test) {

	Type<int> tInt(42);		// integer
	Type<std::string> tStr("Hello World"); // string
	// implicit conversion from double -> float
	Type<float> tFlt(3.1415);
	bool catched = false;

	// Test: Throw an exception if lexical cast fails.
	try {
		// this conversion shouldn't work, since float can't
		// be lexical casted to int.
		Type<int> tError(3.1415);
	} catch(std::bad_cast) {
		catched = true;
	}
	// see if an exception was thrown
	BOOST_CHECK_MESSAGE(catched,
						 "no std::bad_cast exception was thrown from Type<int>(3.1415)");

	// an implicit cast from the string representation
	// should produce a type with the same value if the type
	// is the same (its just a lexical cast like any other).
	Type<int> x(42);
	Type<int> y(x.toString(false));
	
	BOOST_CHECK((int)x == (int)y);
	
	Type<float> f(42.2);
	try {
		// this conversion shouldn't work, since float 
		// can't be lexical casted to int.
		// (even if its encoded as string)
		Type<int> tError(f.toString(false));
	} catch(std::bad_cast) {
		catched = true;
	}

	// see if an exception was thrown
	BOOST_CHECK_MESSAGE(catched,
		"no std::bad_cast exception was thrown");
}

// TestCase toString()
BOOST_AUTO_TEST_CASE(type_toString_test) {

	// create some test dummies
	Type<int> tInt(42);
	Type<float> tFloat(3.1415);
	Type<std::string> tString("Hello World");

	// let's see if the string representation is correct. Remember, that compare
	// returns 0 if everything is fine. So we need to inverse the result for the
	// ASSERT test.
	BOOST_CHECK(!tInt.toString().compare("42"));
	BOOST_CHECK(!tFloat.toString().compare("3.1415"));
	BOOST_CHECK(!tString.toString().compare("Hello World"));

}

// TestCase is()
BOOST_AUTO_TEST_CASE(test_type_is) {

	// create some test dummies
	Type<int> tInt(42);
	Type<float> tFloat(3.1415);
	Type<std::string> tString("Hello World");

	// see if the typeid contains the value expected.
	BOOST_CHECK(tInt.is(typeid(int)));
	BOOST_CHECK(tString.is(typeid(std::string)));
	BOOST_CHECK(tFloat.is(typeid(float)));
}

// TestCase operators()
BOOST_AUTO_TEST_CASE(test_type_operators) {

	// for operations Type<T> should automatically cast to it's internal type and do the operations on it
	Type<int> tInt1(21), tInt2(21);
	BOOST_CHECK(tInt1+tInt2 == Type<int>(42));
	BOOST_CHECK(tInt1*2 == 42);
	BOOST_CHECK(42 - tInt1 == tInt2);
}
