/*
 * typeTest.cpp
 *
 * A unit test suite to confirm the capabilities of the
 * utils::Value class.
 *
 *  Created on: Sep 24, 2009
 *      Author: proeger
 */

// This might crash on MacOS. See http://old.nabble.com/-Boost.Test--Error:-Non-aligned-pointer-being-freed-td24335733.html
// seems to work with XCode 3.2.3 and boost 1.42

#define BOOST_TEST_MODULE ValueTest
#define NOMINMAX 1
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <string>
#include "CoreUtils/type.hpp"
#include "CoreUtils/vector.hpp"
#include <boost/numeric/conversion/converter.hpp>

namespace isis
{
namespace test
{

using util::Value;
using util::fvector4;
using util::ivector4;
using util::_internal::ValueBase;

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( test_type_init )
{
	//  ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, info );
	//  ENABLE_LOG( CoreLog, util::DefaultMsgPrint, info );
	Value<int32_t> tInt( 42 );   // integer
	Value<std::string> tStr( std::string( "Hello World" ) ); // string
	// implicit conversion from double -> float
	Value<float> tFlt( 3.1415 );
	bool catched = false;

	// Test: Throw an exception if lexical cast fails.
	try {
		// this conversion shouldn't work, since float can't
		// be lexical casted to int.
		Value<int32_t> tError( 3.1415 );
	} catch ( std::bad_cast ) {
		catched = true;
	}

	// see if an exception was thrown
	BOOST_CHECK_MESSAGE( catched,
						 "no std::bad_cast exception was thrown from Value<int32_t>(3.1415)" );
	// an implicit cast from the string representation
	// should produce a type with the same value if the type
	// is the same (its just a lexical cast like any other).
	Value<int32_t> x( 42 );
	Value<int32_t> y( x.toString( false ) );
	BOOST_CHECK_EQUAL( ( int32_t )x, ( int32_t )y );
	Value<float> f( 42.2 );

	try {
		// this conversion shouldn't work, since float
		// can't be lexical casted to int.
		// (even if its encoded as string)
		Value<int32_t> tError( f.toString( false ) );
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
	Value<int32_t> tInt( 42 );
	Value<float> tFloat( 3.1415 );
	Value<std::string> tString( std::string( "Hello World" ) );
	BOOST_CHECK_EQUAL( tInt.toString(), "42" );
	BOOST_CHECK_EQUAL( tFloat.toString(), "3.1415" );
	BOOST_CHECK_EQUAL( tString.toString(), "Hello World" );
}

// TestCase is()
BOOST_AUTO_TEST_CASE( test_type_is )
{
	// create some test dummies
	Value<int32_t> tInt( 42 );
	Value<float> tFloat( 3.1415 );
	Value<std::string> tString( std::string( "Hello World" ) );
	// see if the typeid contains the value expected.
	BOOST_CHECK( tInt.is<int32_t>() );
	BOOST_CHECK( tString.is<std::string>() );
	BOOST_CHECK( tFloat.is<float>() );
}

// TestCase operators()
BOOST_AUTO_TEST_CASE( test_type_operators )
{
	// for operations Value<T> should automatically cast to it's internal type and do the operations on it
	Value<int32_t> tInt1( 21 ), tInt2( 21 );
	BOOST_CHECK_EQUAL( tInt1 + tInt2, Value<int32_t>( 42 ) );
	BOOST_CHECK_EQUAL( tInt1 * 2, 42 );
	BOOST_CHECK_EQUAL( 42 - tInt1, tInt2 );
}

BOOST_AUTO_TEST_CASE( type_comparison_test )
{
	Value<uint8_t> _200( ( uint8_t )200 );
	Value<int16_t> _1000( ( int16_t )1000 );
	Value<float> _200komma4( 200.4f );
	Value<float> _200komma6( 200.6f );
	Value<int16_t> _minus1( ( int16_t ) - 1 );
	Value<float> fucking_much( std::numeric_limits<float>::max() );
	Value<double> even_more( std::numeric_limits<double>::max() );
	// this should use implicit conversion to the inner type and compare them
	BOOST_CHECK( _200 < _1000 );
	BOOST_CHECK( _200 > _minus1 );
	BOOST_CHECK( _200.lt( _1000 ) ) ;
	BOOST_CHECK( _200.gt( _minus1 ) ) ;
	//200.4 will be rounded to 200
	BOOST_CHECK( ! _200.lt( _200komma4 ) ) ;
	BOOST_CHECK( _200.eq( _200komma4 ) ) ;
	//200.6 will be rounded to 201
	BOOST_CHECK( _200.lt( _200komma6 ) ) ;
	// kompares 200.4 to 200f
	BOOST_CHECK( ! _200komma4.eq( _200 ) ) ;
	BOOST_CHECK( _200komma4.gt( _200 ) ) ;
	// kompares 200.4 to 200f
	BOOST_CHECK( _200komma4.lt( _1000 ) );
	// push the limits
	BOOST_CHECK( _1000.lt( fucking_much ) );
	BOOST_CHECK( fucking_much.gt( _1000 ) );
	BOOST_CHECK( fucking_much.lt( even_more ) );
}

BOOST_AUTO_TEST_CASE( type_conversion_test )
{
	Value<int32_t> tInt( 42 );
	Value<float> tFloat1( 3.1415 );
	Value<float> tFloat2( 3.5415 );
	Value<ivector4> vec( ivector4( 1, 2, 3, 4 ) );
	ValueBase *iRef = &tInt;
	ValueBase *fRef1 = &tFloat1;
	ValueBase *fRef2 = &tFloat2;
	ValueBase *vRef = &vec;
	BOOST_CHECK_EQUAL( iRef->as<double>(), 42 );
	BOOST_CHECK_EQUAL( fRef1->as<int32_t>(), ( int32_t )ceil( tFloat1 - .5 ) );
	BOOST_CHECK_EQUAL( fRef2->as<int32_t>(), ( int32_t )ceil( tFloat2 - .5 ) );
	BOOST_CHECK_EQUAL( fRef2->as<std::string>(), "3.54150009" );
	BOOST_CHECK_EQUAL( vRef->as<fvector4>(), fvector4( 1, 2, 3, 4 ) );
}

}
}
