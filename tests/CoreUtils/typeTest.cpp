/*
 * typeTest.cpp
 *
 * A unit test suite to confirm the capabilities of the
 * utils::Type class.
 *
 *  Created on: Sep 24, 2009
 *      Author: proeger
 */

// This might crash on MacOS. See http://old.nabble.com/-Boost.Test--Error:-Non-aligned-pointer-being-freed-td24335733.html

#define BOOST_TEST_MODULE TypeTest
#include <boost/test/included/unit_test.hpp>
#include <string>
#include "CoreUtils/type.hpp"
#include "CoreUtils/vector.hpp"
#include <boost/numeric/conversion/converter.hpp>

namespace isis
{
namespace test
{

using util::Type;
using util::fvector4;
using util::ivector4;
using util::_internal::TypeBase;

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( test_type_init )
{
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, info );
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, info );
	Type<int> tInt( 42 );   // integer
	Type<std::string> tStr( std::string( "Hello World" ) ); // string
	// implicit conversion from double -> float
	Type<float> tFlt( 3.1415 );
	bool catched = false;

	// Test: Throw an exception if lexical cast fails.
	try {
		// this conversion shouldn't work, since float can't
		// be lexical casted to int.
		Type<int> tError( 3.1415 );
	} catch ( std::bad_cast ) {
		catched = true;
	}

	// see if an exception was thrown
	BOOST_CHECK_MESSAGE( catched,
						 "no std::bad_cast exception was thrown from Type<int>(3.1415)" );
	// an implicit cast from the string representation
	// should produce a type with the same value if the type
	// is the same (its just a lexical cast like any other).
	Type<int> x( 42 );
	Type<int> y( x.toString( false ) );
	BOOST_CHECK_EQUAL( ( int )x, ( int )y );
	Type<float> f( 42.2 );

	try {
		// this conversion shouldn't work, since float
		// can't be lexical casted to int.
		// (even if its encoded as string)
		Type<int> tError( f.toString( false ) );
	} catch ( std::bad_cast ) {
		catched = true;
	}

	// see if an exception was thrown
	BOOST_CHECK_MESSAGE( catched,
						 "no std::bad_cast exception was thrown" );
}

// TestCase toString()
BOOST_AUTO_TEST_CASE( type_toString_test )
{
	// create some test dummies
	Type<int> tInt( 42 );
	Type<float> tFloat( 3.1415 );
	Type<std::string> tString( std::string( "Hello World" ) );
	BOOST_CHECK_EQUAL( tInt.toString(), "42" );
	BOOST_CHECK_EQUAL( tFloat.toString(), "3.1415" );
	BOOST_CHECK_EQUAL( tString.toString(), "Hello World" );
}

// TestCase is()
BOOST_AUTO_TEST_CASE( test_type_is )
{
	// create some test dummies
	Type<int> tInt( 42 );
	Type<float> tFloat( 3.1415 );
	Type<std::string> tString( std::string( "Hello World" ) );
	// see if the typeid contains the value expected.
	BOOST_CHECK( tInt.is( typeid( int ) ) );
	BOOST_CHECK( tString.is( typeid( std::string ) ) );
	BOOST_CHECK( tFloat.is( typeid( float ) ) );
}

// TestCase operators()
BOOST_AUTO_TEST_CASE( test_type_operators )
{
	// for operations Type<T> should automatically cast to it's internal type and do the operations on it
	Type<int> tInt1( 21 ), tInt2( 21 );
	BOOST_CHECK_EQUAL( tInt1 + tInt2, Type<int>( 42 ) );
	BOOST_CHECK_EQUAL( tInt1*2, 42 );
	BOOST_CHECK_EQUAL( 42 - tInt1, tInt2 );
}

BOOST_AUTO_TEST_CASE( type_conversion_test )
{
	Type<int> tInt( 42 );
	Type<float> tFloat1( 3.1415 );
	Type<float> tFloat2( 3.5415 );
	Type<ivector4> vec( ivector4( 1, 2, 3, 4 ) );
	TypeBase *iRef = &tInt;
	TypeBase *fRef1 = &tFloat1;
	TypeBase *fRef2 = &tFloat2;
	TypeBase *vRef = &vec;
	BOOST_CHECK_EQUAL( iRef->as<double>(), 42 );
	BOOST_CHECK_EQUAL( fRef1->as<int>(), ( int )round( tFloat1 ) );
	BOOST_CHECK_EQUAL( fRef2->as<int>(), ( int )round( tFloat2 ) );
	BOOST_CHECK_EQUAL( fRef2->as<std::string>(), "3.54150009" );
	BOOST_CHECK_EQUAL( vRef->as<fvector4>(), fvector4( 1, 2, 3, 4 ) );
}

}
}
