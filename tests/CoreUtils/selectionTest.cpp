
#define BOOST_TEST_MODULE SelectionTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <string>
#include "CoreUtils/selection.hpp"

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE( test_selection_init )
{
	util::Selection sel( "Val1,Val2,Val3" );
	BOOST_CHECK_EQUAL( sel, "<<NOT_SET>>" );
	BOOST_CHECK_EQUAL( sel, 0 );
	BOOST_CHECK_EQUAL( sel.getEntries().size(), 3 );
	BOOST_CHECK_EQUAL( sel.getEntries(), util::string2list<util::istring>( std::string( "Val1,Val2,Val3" ), ',' ) );
}
BOOST_AUTO_TEST_CASE( test_selection_set )
{
	util::Selection sel( "Val1,Val2,Val3" );
	BOOST_CHECK( ! sel.set( "Val" ) ); //should fail
	BOOST_CHECK( sel.set( "Val1" ) ); //should NOT fail
	BOOST_CHECK_EQUAL( sel, "Val1" );
	BOOST_CHECK_EQUAL( sel, 1 );
}
BOOST_AUTO_TEST_CASE( test_selection_copy )
{
	util::Selection sel( "Val1,Val2,Val3" );
	util::Selection copy = sel;
	BOOST_CHECK( !copy.set( "Val" ) ); //should fail
	BOOST_CHECK( copy.set( "Val1" ) ); //should NOT fail
	BOOST_CHECK_EQUAL( copy, "Val1" );
	BOOST_CHECK_EQUAL( copy, 1 );
	sel = copy;
	BOOST_CHECK_EQUAL( sel, "Val1" );
	BOOST_CHECK_EQUAL( sel, 1 );
}
}
}