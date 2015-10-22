/*
 * imageTest.cpp
 *
 *  Created on: Oct 1, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE byteswapTest
#include <boost/test/unit_test.hpp>
#include "data/endianess.hpp"

namespace isis
{
namespace test
{
BOOST_AUTO_TEST_CASE ( byteswap_test )//Copy chunks
{
	BOOST_CHECK_EQUAL( data::endianSwap<uint16_t>( 0xF1F2 ),                         0xF2F1 );
	BOOST_CHECK_EQUAL( data::endianSwap<uint32_t>( 0xF1F2F3F4 ),                 0xF4F3F2F1 );
	BOOST_CHECK_EQUAL( data::endianSwap<uint64_t>( 0xF1F2F3F4F5F6F7F8 ), 0xF8F7F6F5F4F3F2F1 );

	float  fone, finv;
	double done, dinv;
	reinterpret_cast<uint32_t &>( fone ) = 0xF1F2F3F4;
	reinterpret_cast<uint64_t &>( done ) = 0xF1F2F3F4F5F6F7F8;
	reinterpret_cast<uint32_t &>( finv ) =        0xF4F3F2F1;
	reinterpret_cast<uint64_t &>( dinv ) = 0xF8F7F6F5F4F3F2F1;

	BOOST_CHECK_EQUAL( data::endianSwap( fone ), finv );
	BOOST_CHECK_EQUAL( data::endianSwap( done ), dinv );
}
}
}
