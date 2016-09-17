/*
* A unit test suite to confirm the capabilities of the
* utils::Value class.
*
*  Created on: Oct 19, 2009
*      Author: reimer
*/

#define BOOST_TEST_MODULE VectorTest
#include <boost/test/unit_test.hpp>
#include <isis/util/vector.hpp>
#include <isis/util/matrix.hpp>
#include <isis/math/transform.hpp>

#include <string.h>

namespace isis
{
namespace test
{

using util::fvector4;

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( vector_init_test )
{
	util::FixedVector<float,4> test({1,1,1,1}); // @todo get rid of outer brackets (and the explicit constructor) if c++17 is available
	float *start = &test[0];
	const float compare1[] = { 1,  1,  1,  1};
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

BOOST_AUTO_TEST_CASE( matrix_init_test )
{
	const uint8_t buff[] = {
		0, 1, 2,
		3, 4, 5,
		6, 7, 8,
		9, 10, 11
	};
	util::FixedMatrix<uint8_t, 3, 4> test( buff );

	for( int i = 0; i < 3; i++ )
		for( int j = 0; j < 4; j++ )
			BOOST_CHECK_EQUAL( test.elem( i, j ), i + j * 3 );
}

BOOST_AUTO_TEST_CASE( matrix4x4_init_test )
{
	util::vector4<uint8_t> b1( {0, 1, 2, 3} );
	util::vector4<uint8_t> b2( {4, 5, 6, 7} );
	util::vector4<uint8_t> b3( {8, 9, 10, 11} );
	util::vector4<uint8_t> b4( {12, 13, 14, 15} );


	util::Matrix4x4<uint8_t> test( {b1, b2, b3, b4} );

	for( int i = 0; i < 4; i++ )
		for( int j = 0; j < 4; j++ )
			BOOST_CHECK_EQUAL( test.elem( i, j ), i + j * 4 );
}

BOOST_AUTO_TEST_CASE( matrix_dot_test )
{
	util::vector4<float> b1( {1 / sqrtf( 2 ), -1 / sqrtf( 2 )} );
	util::vector4<float> b2( {1 / sqrtf( 2 ),  1 / sqrtf( 2 )} );

	util::Matrix4x4<float> test( b1, b2 );

	float E[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	// an orthogonal matrix mult with its transpose is E
	util::Matrix4x4<float> d = test.dot( test.transpose() );
	BOOST_CHECK( d.fuzzyEqual( util::Matrix4x4<float>( E ) ) );

	// applying two transformations on a vector has the same result as applying their "dot" on the vector
	BOOST_CHECK(
		test.dot( test ).dot( fvector4( {1, 2} ) ).fuzzyEqual( test.dot( test.dot( fvector4( {1, 2} ) ) ) )
	);
}

BOOST_AUTO_TEST_CASE( matrix_from_boost_to_boost )
{
	using namespace boost::numeric::ublas;
	const size_t m = 10;
	const size_t n = 7;
	matrix<float> boost_matrix = matrix<float>( m, n );
	unsigned short index = 0;

	for ( unsigned short i = 0; i < m; i++ ) {
		for ( unsigned short j = 0; j < n; j++ ) {
			boost_matrix( i, j ) = index++;
		}
	}

	isis::util::FixedMatrix<float, n, m> isis_matrix= math::fromBoostMatrix<float,n,m>(boost_matrix);

	for ( unsigned short i = 0; i < m; i++ ) {
		for ( unsigned short j = 0; j < n; j++ ) {
			const matrix<float> dummy= math::toBoostMatrix(isis_matrix);
			BOOST_CHECK_EQUAL( boost_matrix( i, j ), isis_matrix.elem( j, i ) );
			BOOST_CHECK_EQUAL( boost_matrix( i, j ), dummy(i,j) );
		}
	}
}

BOOST_AUTO_TEST_CASE( matrix_inverse )
{
	isis::util::IdentityMatrix<float, 3> identity_matrix;
	isis::util::FixedMatrix<float, 3, 3> initial_matrix;
	initial_matrix.elem( 0, 0 ) = 0;
	initial_matrix.elem( 1, 0 ) = 1;
	initial_matrix.elem( 2, 0 ) = 0;
	initial_matrix.elem( 0, 1 ) = 1;
	initial_matrix.elem( 1, 1 ) = 0;
	initial_matrix.elem( 2, 1 ) = 0;
	initial_matrix.elem( 0, 2 ) = 0;
	initial_matrix.elem( 1, 2 ) = 0;
	initial_matrix.elem( 2, 2 ) = 1;
	bool ok;
	isis::util::FixedMatrix<float, 3, 3> inverse = math::inverseMatrix(initial_matrix, ok );
	BOOST_CHECK( ok );
	isis::util::FixedMatrix<float, 3, 3> result = math::fromBoostMatrix<float,3,3>(
			boost::numeric::ublas::prod( math::toBoostMatrix(inverse), math::toBoostMatrix(initial_matrix))
	);

	for( unsigned short i = 0; i < 3; i++ ) {
		for( unsigned short j = 0; j < 3; j++ ) {
			BOOST_CHECK_EQUAL( result.elem( i, j ), identity_matrix.elem( i , j ) );
		}
	}


}

}
}
