/*
* A unit test suite to confirm the capabilities of the
* utils::Value class.
*
*  Created on: Oct 19, 2009
*      Author: reimer
*/

#define BOOST_TEST_MODULE VectorTest
#include <boost/test/unit_test.hpp>
#include "../../isis/util/vector.hpp"
#include "../../isis/util/matrix.hpp"
#include "../../isis/math/transform.hpp"

#include <string.h>
#include <cmath>

namespace isis
{
namespace test
{

using util::fvector4;

// TestCase object instantiation
BOOST_AUTO_TEST_CASE( vector_init_test )
{
	std::array<float,4> test{1,1,1,1}; // @todo get rid of outer brackets (and the explicit constructor) if c++17 is available
	float *start = &test[0];
	const float compare1[] = { 1,  1,  1,  1};
	const float compare2[] = {42, 42, 42, 42};
	BOOST_CHECK( memcmp( start, compare1, sizeof( float ) * 4 ) == 0 );
	test.fill( 42 );
	BOOST_CHECK( memcmp( start, compare2, sizeof( float ) * 4 ) == 0 );
}

BOOST_AUTO_TEST_CASE( vector_output_test )
{
	fvector4 test1{}, test2;
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
	fvector4 test1{}, test2{};
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
	util::Matrix<uint8_t, 3, 4> test{
		0,  1,  2,
		3,  4,  5,
		6,  7,  8,
		9, 10, 11
	};

	for( int c = 0; c < 3; c++ )
		for( int r = 0; r < 4; r++ )
			BOOST_CHECK_EQUAL( test[r][c], c + r * 3 );
}

BOOST_AUTO_TEST_CASE( matrix4x4_init_test )
{
	const util::vector4<uint8_t> 
		b1{0, 1, 2, 3},
		b2{4, 5, 6, 7},
		b3{8, 9, 10, 11},
		b4{12, 13, 14, 15};

	util::Matrix4x4<uint8_t> test{b1, b2, b3, b4};

	for( int c = 0; c < 4; c++ )
		for( int r = 0; r < 4; r++ )
			BOOST_CHECK_EQUAL( test[r][c], c + r * 4 );
}

BOOST_AUTO_TEST_CASE( matrix_dot_test )
{
	util::Matrix4x4<float> test{
		1 / sqrtf( 2 ), -1 / sqrtf( 2 ),0,0,
		1 / sqrtf( 2 ),  1 / sqrtf( 2 ),0,0,
		0,               0,             1,0,
		0,               0,             0,1
	};

	util::Matrix4x4<float> E{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	// an orthogonal matrix mult with its transpose is E
	BOOST_CHECK( util::fuzzyEqualM( 
		test*util::transpose(test), 
		util::identityMatrix<float,4>() 
	));
}
BOOST_AUTO_TEST_CASE( matrix_vector_dot_test )
{
	const float cos=std::cos(M_PI_2);
	const float sin=std::sin(M_PI_2);
	util::Matrix3x3<float> rot{
		cos, -sin,0,
		sin,  cos,0,
		0,    0,  1
	};
	
	BOOST_CHECK_EQUAL(
		(rot * util::vector3<float>{1, 2, 0}),
		(util::vector3<float>{-2, 1, 0})
	);

// 	// applying two transformations on a vector has the same result as applying their "dot" on the vector
// 	BOOST_CHECK(
// 		util::fuzzyEqualV( (test * test ) * fvector4{1, 2, 0 ,0}, test*( test * fvector4{1, 2, 0, 0} ) )
// 	);
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

	isis::util::Matrix<float, n, m> isis_matrix= math::fromBoostMatrix<float,n,m>(boost_matrix);

	for ( unsigned short i = 0; i < m; i++ ) {
		for ( unsigned short j = 0; j < n; j++ ) {
			const matrix<float> dummy= math::toBoostMatrix(isis_matrix);
			BOOST_CHECK_EQUAL( boost_matrix( i, j ), isis_matrix[j][i] );
			BOOST_CHECK_EQUAL( boost_matrix( i, j ), dummy(i,j) );
		}
	}
}

BOOST_AUTO_TEST_CASE( matrix_inverse )
{
	auto identity_matrix = isis::util::identityMatrix<float, 3>();
	isis::util::Matrix3x3<float> initial_matrix;
	initial_matrix[0][0] = 0;
	initial_matrix[1][0] = 1;
	initial_matrix[2][0] = 0;
	initial_matrix[0][1] = 1;
	initial_matrix[1][1] = 0;
	initial_matrix[2][1] = 0;
	initial_matrix[0][2] = 0;
	initial_matrix[1][2] = 0;
	initial_matrix[2][2] = 1;
	bool ok;
	isis::util::Matrix<float, 3, 3> inverse = math::inverseMatrix(initial_matrix, ok );
	BOOST_CHECK( ok );
	isis::util::Matrix<float, 3, 3> result = math::fromBoostMatrix<float,3,3>(
			boost::numeric::ublas::prod( math::toBoostMatrix(inverse), math::toBoostMatrix(initial_matrix))
	);

	for( unsigned short i = 0; i < 3; i++ ) {
		for( unsigned short j = 0; j < 3; j++ ) {
			BOOST_CHECK_EQUAL( result[i][j], identity_matrix[i][j] );
		}
	}


}

}
}
