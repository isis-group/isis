/*
* propertyTest.cpp
*
*  Created on: Sep 23, 2009
*      Author: proeger
*/

// The BOOST_TEST_MODULE is similar to a unit test suite.
#define BOOST_TEST_MODULE PropMapTests
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <CoreUtils/propmap.hpp>
#include <CoreUtils/vector.hpp>
#include <string>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE( propMap_init_test )
{
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	util::PropMap map1;
	map1.propertyValue( "Test1" ) = 6.4;
	map1.propertyValue( "Test2" ) = ( int32_t )5;
	map1.propertyValue( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map1.propertyValue( "Test4" ) = std::string( "Hallo" );
	map1.propertyValue( "sub/Test1" ) = ( int32_t )1;
	map1.propertyValue( "sub/Test2" ) = ( int32_t )2;
	BOOST_CHECK_EQUAL( map1.propertyValue( "Test1" ), 6.4 );
	BOOST_CHECK_EQUAL( map1.propertyValue( "Test2" ), ( int32_t )5 );
	BOOST_CHECK_EQUAL( map1.propertyValue( "Test3" ), util::fvector4( 1, 1, 1, 1 ) );
	BOOST_CHECK_EQUAL( map1.propertyValue( "Test4" ), std::string( "Hallo" ) );
	util::PropMap &ref = map1.branch( "sub" );
	BOOST_CHECK( ! ref.empty() );
	BOOST_CHECK_EQUAL( ref.propertyValue( "Test1" ), ( int32_t )1 );
	BOOST_CHECK_EQUAL( ref.propertyValue( "Test2" ), ( int32_t )2 );
	BOOST_CHECK( map1.propertyValue( "new" ).empty() );
}

BOOST_AUTO_TEST_CASE( propMap_set_test )
{
	util::PropMap map1;
	map1.propertyValue( "Test1" ) = 6.4;
	BOOST_CHECK( !( map1.propertyValue( "Test1" ) == 7 ) );
	BOOST_CHECK_EQUAL( map1.propertyValue( "Test1" ), 6.4 );
	map1.setProperty( "Test1", 7. );
	BOOST_CHECK_EQUAL( map1.propertyValue( "Test1" ), 7 );
}

BOOST_AUTO_TEST_CASE( propMap_remove_test )
{
	util::PropMap map;
	map.propertyValue( "Test1" ) = 6.4;
	map.propertyValue( "Test2" ) = ( int32_t )5;
	map.propertyValue( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map.propertyValue( "Test4" ) = std::string( "Hallo" );
	map.propertyValue( "sub/Test1" ) = ( int32_t )1;
	map.propertyValue( "sub/Test2" ) = ( int32_t )2;
	BOOST_CHECK( map.remove( "Test1" ) );
	BOOST_CHECK( map.remove( "Test2" ) );
	BOOST_CHECK( map.remove( "Test3" ) );
	BOOST_CHECK( map.remove( "Test4" ) );
	BOOST_CHECK( map.remove( "sub/Test1" ) );
	BOOST_CHECK( ! map.branch( "sub" ).empty() ); //the submap must still be there
	BOOST_CHECK( map.remove( "sub/Test2" ) );
	BOOST_CHECK( map.branch( "sub" ).empty() ); //not anymore (this will create an "normal" empty entry)
}

BOOST_AUTO_TEST_CASE( propMap_join_test )
{
	util::PropMap map1, map2, result, org;
	map1.propertyValue( "Test1" ) = 6.4;
	map1.propertyValue( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map1.propertyValue( "Test4" ) = std::string( "Hallo" );
	util::PropMap::key_list rej;
	//create empty Property "Test5" through accessing it
	BOOST_CHECK( map1.propertyValue( "Test5" ).empty() );
	map2.propertyValue( "Test2" ) = ( int32_t )5;
	map2.propertyValue( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map2.propertyValue( "Test4" ) = std::string( "Hallo Welt" );
	map2.propertyValue( "Test5" ) = std::string( "nix leer" );
	//First check not overwriting join
	result = map1;
	rej = result.join( map2 );
	// Test2 should be inserted because its in map1
	// Test5 should be inserted because its empty in map1
	org = map1;
	org.propertyValue( "Test2" ) = map2.propertyValue( "Test2" );
	org.propertyValue( "Test5" ) = map2.propertyValue( "Test5" );
	BOOST_CHECK_EQUAL( org, result );
	// There should be one rejected key in reject, namely Test4 (its allready set in map1)
	BOOST_CHECK_EQUAL( rej.size(), ( int32_t )1 );
	BOOST_CHECK_EQUAL( *rej.begin(), "Test4" );
	//now check with overwriting
	result = map1;
	rej = result.join( map2, true );
	//Test4 should be overwritten, and therefore be set to  "Hallo Welt"
	org.propertyValue( "Test4" ) = std::string( "Hallo Welt" );
	BOOST_CHECK_EQUAL( org, result );
	// nothing should be rejected
	BOOST_CHECK( rej.empty() );
}

BOOST_AUTO_TEST_CASE( propMap_diff_test )
{
	util::PropMap map1, map2;
	map1.setProperty( "Test1", 6.4 );
	map1.setProperty( "Test3", util::fvector4( 1, 1, 1, 1 ) );
	map1.setProperty( "Test4", std::string( "Hallo" ) );
	map1.setProperty( "Test6/1", std::string( "nix" ) );
	map1.setProperty( "Test6/2", std::string( "leer" ) );
	util::PropMap::key_list rej;
	//create empty Property "Test5" through accessing it
	BOOST_CHECK( map1.propertyValue( "Test5" ).empty() );
	map2.setProperty( "Test2", ( int32_t )5 );
	map2.setProperty( "Test3", util::fvector4( 1, 1, 1, 1 ) );
	map2.setProperty( "Test4", std::string( "Hallo Welt" ) );
	map2.setProperty( "Test5", std::string( "Hallo leer" ) );
	map2.setProperty( "Test6", std::string( "Hallo branch" ) );
	util::PropMap::diff_map result = map1.getDifference( map2 ), org;
	//Test1 must be pair of map1.propertyValue("Test1") and |empty|
	BOOST_CHECK_EQUAL( result["Test1"].first, map1.propertyValue( "Test1" ) );
	BOOST_CHECK( result["Test1"].second.empty() );
	//Test2 must be pair of |empty| and map2.propertyValue("Test2")
	BOOST_CHECK( result["Test2"].first.empty() );
	BOOST_CHECK_EQUAL( result["Test2"].second, map2.propertyValue( "Test2" ) );
	//Test2 must be pair of map1.propertyValue("Test4") and map2.propertyValue("Test4")
	BOOST_CHECK_EQUAL( result["Test4"].first, map1.propertyValue( "Test4" ) );
	BOOST_CHECK_EQUAL( result["Test4"].second, map2.propertyValue( "Test4" ) );
	//Test5 must be pair of |empty| and map2.propertyValue("Test5")
	BOOST_CHECK( result["Test5"].first.empty() );
	BOOST_CHECK_EQUAL( result["Test5"].second, map2.propertyValue( "Test5" ) );
}
BOOST_AUTO_TEST_CASE( propMap_transform_test )
{
	util::PropMap map;
	map.propertyValue( "Test1" ) = 6.4;
	map.propertyValue( "Test2" ) = ( int32_t )5;
	map.propertyValue( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map.propertyValue( "Test4" ) = std::string( "Hallo" );
	map.propertyValue( "sub/Test1" ) = ( int32_t )1;
	map.propertyValue( "sub/Test2" ) = ( int32_t )2;
	BOOST_CHECK( map.transform<float>( "sub/Test1", "sub/Test1float" ) );
	BOOST_CHECK( map.transform<int32_t>( "Test1", "Test1int" ) );
	BOOST_CHECK( map.transform<util::ivector4>( "Test3", "Test3int" ) );
	BOOST_CHECK( map.propertyValue( "sub/Test1float" )->is<float>() );
	BOOST_CHECK( map.propertyValue( "Test1int" )->is<int>() );
	BOOST_CHECK( map.propertyValue( "Test3int" )->is<util::ivector4>() );
	BOOST_CHECK_EQUAL( map.propertyValue( "sub/Test1float" ), ( float )1 );
	BOOST_CHECK_EQUAL( map.propertyValue( "sub/Test1float" ), ( int32_t )1 ); // this will do an automatic transform back to int for comparison
	BOOST_CHECK_EQUAL( map.propertyValue( "Test1int" ), ( int32_t )6 );
	BOOST_CHECK_EQUAL( map.propertyValue( "Test3int" ), util::ivector4( 1, 1, 1, 1 ) );
}
}
}