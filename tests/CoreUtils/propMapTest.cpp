/*
* propertyTest.cpp
*
*  Created on: Sep 23, 2009
*      Author: proeger
*/

// The BOOST_TEST_MODULE is similar to a unit test suite.
#define BOOST_TEST_MODULE PropMapTests
#include <boost/test/unit_test.hpp>
#include <CoreUtils/propmap.hpp>
#include <CoreUtils/vector.hpp>
#include <string>

namespace isis
{
namespace test
{

using util::PropertyMap;

BOOST_AUTO_TEST_CASE( propPath_test )
{

	const PropertyMap::PropPath a( "a" ), bc( "b/c" ), abc( "a/b/c" );
	//creation
	BOOST_CHECK_EQUAL( PropertyMap::PropPath( "a/a/a" ), std::list<PropertyMap::key_type>( 3, PropertyMap::key_type( "a" ) ) );

	//combination
	BOOST_CHECK_EQUAL( a / bc, abc );

	//length
	BOOST_CHECK_EQUAL( abc.length(), boost::lexical_cast<std::string>( abc ).length() );
}

BOOST_AUTO_TEST_CASE( propMap_init_test )
{
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	PropertyMap map1;
	map1.property( "Test1" ) = 6.4;
	map1.property( "Test2" ) = ( int32_t )5;
	map1.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map1.property( "Test4" ) = std::string( "Hallo" );
	map1.property( "sub/Test1" ) = ( int32_t )1;
	map1.property( "sub/Test2" ) = ( int32_t )2;
	BOOST_CHECK_EQUAL( map1.property( "Test1" ), 6.4 );
	BOOST_CHECK_EQUAL( map1.property( "Test2" ), ( int32_t )5 );
	BOOST_CHECK_EQUAL( map1.property( "Test3" ), util::fvector4( 1, 1, 1, 1 ) );
	BOOST_CHECK_EQUAL( map1.property( "Test4" ), std::string( "Hallo" ) );
	PropertyMap &ref = map1.branch( "sub" );
	BOOST_CHECK( ! ref.isEmpty() );
	BOOST_CHECK_EQUAL( ref.property( "Test1" ), ( int32_t )1 );
	BOOST_CHECK_EQUAL( ref.property( "Test2" ), ( int32_t )2 );
	BOOST_CHECK( map1.property( "new" ).isEmpty() );
}

BOOST_AUTO_TEST_CASE( propMap_set_test )
{
	PropertyMap map1;
	map1.property( "Test1" ) = 6.4;
	BOOST_CHECK( !( map1.property( "Test1" ) == 7 ) );
	BOOST_CHECK_EQUAL( map1.property( "Test1" ), 6.4 );
	map1.setValueAs( "Test1", 7. );
	BOOST_CHECK_EQUAL( map1.property( "Test1" ), 7 );

	map1.setValueAs<bool>( "bool", true );
	BOOST_CHECK_EQUAL( map1.property( "bool" ), true );
	map1.setValueAs<bool>( "bool", false );
	BOOST_CHECK_EQUAL( map1.property( "bool" ), false );

}

BOOST_AUTO_TEST_CASE( propMap_remove_test )
{
	PropertyMap map;
	map.property( "Test1" ) = 6.4;
	map.property( "Test2" ) = ( int32_t )5;
	map.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map.property( "Test4" ) = std::string( "Hallo" );
	map.property( "sub/Test1" ) = ( int32_t )1;
	map.property( "sub/Test2" ) = ( int32_t )2;
	BOOST_CHECK( map.remove( "Test1" ) );
	BOOST_CHECK( map.remove( "Test2" ) );
	BOOST_CHECK( map.remove( "Test3" ) );
	BOOST_CHECK( map.remove( "Test4" ) );
	BOOST_CHECK( map.remove( "sub/Test1" ) );
	BOOST_CHECK( ! map.branch( "sub" ).isEmpty() ); //Test2 is still there
	BOOST_CHECK( map.remove( "sub/Test2" ) ); // this will also delete sub
	BOOST_CHECK( ! map.hasBranch( "sub" ) ); // so its not there anymore
}

BOOST_AUTO_TEST_CASE( propMap_join_test )
{
	PropertyMap map1, map2, result, org;
	map1.property( "Test1" ) = 6.4;
	map1.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map1.property( "Test4" ) = std::string( "Hallo" );
	
	//create empty Property "Test5" through accessing it
	BOOST_CHECK( map1.property( "Test5" ).isEmpty() );
	map2.property( "Test2" ) = ( int32_t )5;
	map2.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map2.property( "Test4" ) = std::string( "Hallo Welt" );
	map2.property( "Test5" ) = std::string( "nix leer" );
	//First check not overwriting join
	result = map1;
	PropertyMap::PathSet rej = result.join( map2 );
	// Test2 should be inserted because its in map1
	// Test5 should be inserted because its empty in map1
	org = map1;
	org.property( "Test2" ) = map2.property( "Test2" );
	org.property( "Test5" ) = map2.property( "Test5" );
	BOOST_CHECK_EQUAL( org, result );
	// There should be one rejected key in reject, namely Test4 (its allready set in map1)
	// Test3 is equal in both thus its not really "rejected"
	BOOST_CHECK_EQUAL( rej.size(), 1 );
	BOOST_CHECK_EQUAL( *rej.begin(), "Test4" );

	//now check with overwriting
	result = map1;
	BOOST_CHECK_EQUAL( result.property( "Test4" ), std::string( "Hallo" ) );

	rej = result.join( map2, true );
	//Test4 should be overwritten, and therefore be set to  "Hallo Welt" now
	BOOST_CHECK_EQUAL( result.property( "Test4" ), std::string( "Hallo Welt" ) );
	// nothing should be rejected
	BOOST_CHECK( rej.empty() );
}

BOOST_AUTO_TEST_CASE( propMap_diff_test )
{
	PropertyMap map1, map2;
	map1.setValueAs( "Test1", 6.4 );
	map1.setValueAs( "Test3", util::fvector4( 1, 1, 1, 1 ) );
	map1.setValueAs( "Test4", std::string( "Hallo" ) );
	map1.setValueAs( "Test6/1", std::string( "nix" ) );
	map1.setValueAs( "Test6/2", std::string( "leer" ) );
	PropertyMap::PathSet rej;
	//create empty Property "Test5" through accessing it
	BOOST_CHECK( map1.property( "Test5" ).isEmpty() );
	map2.setValueAs( "Test2", ( int32_t )5 );
	map2.setValueAs( "Test3", util::fvector4( 1, 1, 1, 1 ) );
	map2.setValueAs( "Test4", std::string( "Hallo Welt" ) );
	map2.setValueAs( "Test5", std::string( "Hallo leer" ) );
	map2.setValueAs( "Test6", std::string( "Hallo branch" ) );
	PropertyMap::DiffMap result = map1.getDifference( map2 ), org;
	//Test1 must be pair of map1.property("Test1") and |empty|
	BOOST_CHECK_EQUAL( result["Test1"].first, map1.property( "Test1" ) );
	BOOST_CHECK( result["Test1"].second.isEmpty() );
	//Test2 must be pair of |empty| and map2.property("Test2")
	BOOST_CHECK( result["Test2"].first.isEmpty() );
	BOOST_CHECK_EQUAL( result["Test2"].second, map2.property( "Test2" ) );
	//Test2 must be pair of map1.property("Test4") and map2.property("Test4")
	BOOST_CHECK_EQUAL( result["Test4"].first, map1.property( "Test4" ) );
	BOOST_CHECK_EQUAL( result["Test4"].second, map2.property( "Test4" ) );
	//Test5 must be pair of |empty| and map2.property("Test5")
	BOOST_CHECK( result["Test5"].first.isEmpty() );
	BOOST_CHECK_EQUAL( result["Test5"].second, map2.property( "Test5" ) );
}

BOOST_AUTO_TEST_CASE( propMap_rename_test )
{
	PropertyMap map;
	map.property( "Test1" ) = 6.4;
	map.property( "Test2" ) = ( int32_t )5;
	map.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map.property( "Test4" ) = std::string( "Hallo" );
	map.property( "sub/Test1" ) = ( int32_t )1;
	map.property( "sub/Test2" ) = ( int32_t )2;

	// property rename
	BOOST_CHECK( map.rename("Test1","Test11") );
	BOOST_CHECK( !map.hasProperty("Test1") ); //should be gone
	BOOST_CHECK_EQUAL( map.property("Test11"), 6.4); // .. be here actually

	BOOST_CHECK( !map.rename("Test2","Test3") );//should not rename into existing stuff
	BOOST_CHECK_EQUAL( map.property("Test2"), ( int32_t )5); //should still be there
	BOOST_CHECK_EQUAL( map.property("Test3"), util::fvector4( 1, 1, 1, 1 )); // as well as this

	// tree rename
	BOOST_CHECK( map.rename("sub","sup") );
	BOOST_CHECK( !map.hasBranch("sub")); //should be gone
	BOOST_CHECK_EQUAL( map.property("sup/test1"), ( int32_t )1); // subtree should be here
	BOOST_CHECK_EQUAL( map.property("sup/test2"), ( int32_t )2); // subtree should be here

	BOOST_CHECK_EQUAL(boost::lexical_cast<std::string>(map.branch("sup")),"{Test1:1(s32bit),Test2:2(s32bit)}"); // first check stringinfy
	BOOST_CHECK( map.rename("sup/Test1","sup/TEST1") ); // lexical rename (actually the case of the source doesn't matter)
	BOOST_CHECK_EQUAL(boost::lexical_cast<std::string>(map.branch("sup")),"{TEST1:1(s32bit),Test2:2(s32bit)}"); // should be uppercase now
}


BOOST_AUTO_TEST_CASE( propMap_transform_test )
{
	PropertyMap map;
	map.property( "Test1" ) = 6.4;
	map.property( "Test2" ) = ( int32_t )5;
	map.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map.property( "Test4" ) = std::string( "Hallo" );
	map.property( "sub/Test1" ) = ( int32_t )1;
	map.property( "sub/Test2" ) = ( int32_t )2;
	BOOST_CHECK( map.transform<float>( "sub/Test1", "sub/Test1float" ) );
	BOOST_CHECK( map.transform<int32_t>( "Test1", "Test1int" ) );
	BOOST_CHECK( map.transform<util::ivector4>( "Test3", "Test3int" ) );
	BOOST_CHECK( map.property( "sub/Test1float" ).is<float>() );
	BOOST_CHECK( map.property( "Test1int" ).is<int>() );
	BOOST_CHECK( map.property( "Test3int" ).is<util::ivector4>() );
	BOOST_CHECK_EQUAL( map.property( "sub/Test1float" ), ( float )1 );
	BOOST_CHECK_EQUAL( map.property( "sub/Test1float" ), ( int32_t )1 ); // this will do an automatic transform back to int for comparison
	BOOST_CHECK_EQUAL( map.property( "Test1int" ), ( int32_t )6 );
	BOOST_CHECK_EQUAL( map.property( "Test3int" ), util::ivector4( 1, 1, 1, 1 ) );
}

BOOST_AUTO_TEST_CASE( propMap_find_test )
{
	PropertyMap map;
	map.property( "Test1" ) = 6.4;
	map.property( "Test2" ) = ( int32_t )5;
	map.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map.property( "Test4" ) = std::string( "Hallo" );
	map.property( "sub1/subProp1" ) = ( int32_t )1;
	map.property( "sub2/subProp2" ) = ( int32_t )2;
	map.property( "sub2/sub1/subsubProp2" ) = ( int32_t )2;

	BOOST_CHECK_EQUAL( map.find( "subProp1" ), "sub1/subProp1" );
	BOOST_CHECK_EQUAL( map.find( "subProp2" ), "sub2/subProp2" );
	BOOST_CHECK( map.find( "sub" ).empty() );

	map.property( "sub1/sub1" ) = ( int32_t )1;
	BOOST_CHECK_EQUAL( map.find( "sub1" ), "sub1/sub1" );
	BOOST_CHECK_EQUAL( map.find( "sub1", false, true ), "sub1" ); // this is the branch "sub1"
}

BOOST_AUTO_TEST_CASE( prop_list_test )
{

	PropertyMap map;
	const util::Value<int> five[] = {1, 2, 3, 4, 5};
	std::copy( five, five + 5, std::back_inserter( map.property( "test1" ) ) );

	for( int i = 0; i < 5; i++ ) {
		BOOST_CHECK_EQUAL( map.getValueAs<int>( "test1", i ), five[i] );
		BOOST_CHECK_EQUAL( map.property( "test1" )[i], five[i] );
		map.property( "test2" ).push_back( five[i] ); //adding same values to prop test2
	}

	BOOST_CHECK_EQUAL( map.property( "test1" ), map.property( "test2" ) ); //so both should be equal

	for( int i = 4; i >= 0; i-- ) { // filling / setting from the back
		map.setValueAs<int>( "test1back", five[i], i );
	}

	BOOST_CHECK_EQUAL( map.property( "test1" ), map.property( "test1back" ) ); //again both should be equal

	map.setValueAs<int>( "test1back", 10, 10 );
	BOOST_CHECK_NE( map.property( "test1" ), map.property( "test1back" ) ); //no anymore
}

BOOST_AUTO_TEST_CASE( prop_list_splice_test )
{

	PropertyMap map;
	const util::Value<int> buff[] = {1, 2, 3, 4, 5, 6};
	std::copy( buff, buff + 6, std::back_inserter( map.property( "test1" ) ) );
	std::copy( buff, buff + 6, std::back_inserter( map.property( "test2" ) ) );

	map.setValueAs<std::string>( "Test3", "Hallo" );
	map.setValueAs<std::string>( "Test4", "Welt" );

	std::vector<PropertyMap> dst( 6 );
	for( const PropertyMap & ref :  dst )
	BOOST_REQUIRE( ref.isEmpty() );
	map.splice( dst.begin(), dst.end(), true );

	int i = 0;
	for( const PropertyMap & ref :  dst ) {
		// all must be scalar now
		BOOST_CHECK_EQUAL( ref.property( "test1" ).size(), 1 );
		BOOST_CHECK_EQUAL( ref.property( "test2" ).size(), 1 );
		BOOST_CHECK_EQUAL( ref.property( "test3" ).size(), 1 );
		BOOST_CHECK_EQUAL( ref.property( "test4" ).size(), 1 );
		// test1/2 must be spliced
		BOOST_CHECK_EQUAL( ref.property( "test1" ), buff[i] );
		BOOST_CHECK_EQUAL( ref.property( "test2" ), buff[i] );
		i++;
	}

	dst = std::vector<PropertyMap>( 3 );
	map.splice( dst.begin(), dst.end(), true );
	i = 0;
	for( const PropertyMap & ref :  dst ) {
		// all must be scalar now
		BOOST_CHECK_EQUAL( ref.property( "test1" ).size(), 2 );
		BOOST_CHECK_EQUAL( ref.property( "test2" ).size(), 2 );
		BOOST_CHECK_EQUAL( ref.property( "test3" ).size(), 1 );
		BOOST_CHECK_EQUAL( ref.property( "test4" ).size(), 1 );
		// test1/2 must be spliced
		BOOST_CHECK_EQUAL( ref.property( "test1" )[0], buff[i * 2] );
		BOOST_CHECK_EQUAL( ref.property( "test2" )[0], buff[i * 2] );
		BOOST_CHECK_EQUAL( ref.property( "test1" )[1], buff[i * 2 + 1] );
		BOOST_CHECK_EQUAL( ref.property( "test2" )[1], buff[i * 2 + 1] );
		i++;
	}
}

BOOST_AUTO_TEST_CASE( prop_value_ref_test )
{
	PropertyMap map;
	map.setValueAs( "Test1", util::fvector3( 1, 2, 3 ) );
	BOOST_CHECK_EQUAL( map.property( "Test1" ), util::fvector3( 1, 2, 3 ) );

	BOOST_CHECK( map.refValueAs<util::fvector3>( "Test1" ) ); //should be available as fvector3

	optional< util::fvector4 & > vec4 = map.refValueAs<util::fvector4>( "Test1" );
	BOOST_CHECK( vec4 ); //should be available as fvector4 (conversion available)

	// changes should affect the property
	vec4.get()[3] = 4;
	BOOST_CHECK_EQUAL( map.getValueAs<util::fvector4>( "Test1" ), util::fvector4( 1, 2, 3, 4 ) );
}

BOOST_AUTO_TEST_CASE( propMap_transfer_test )
{
	PropertyMap map1,map2;
	map1.property( "Test1" ) = M_2_PI;
	map1.property( "Test2" ) = ( int32_t )5;
	map1.property( "Test3" ) = util::fvector4( 1, 1, 1, 1 );
	map1.property( "Test4" ) = std::string( "Hallo" );
	map1.property( "sub/Test1" ) = ( int32_t )1;
	map1.property( "sub/Test2" ) = ( int32_t )2;

	BOOST_CHECK(map2.transfer(map1).empty());
	BOOST_CHECK(map1.isEmpty()); //source should be empty after transfer

	// stuff should be in map2 now
	BOOST_CHECK_EQUAL(map2.property( "Test1" ), M_2_PI);
	BOOST_CHECK_EQUAL(map2.property( "Test2" ), ( int32_t )5);
	BOOST_CHECK_EQUAL(map2.property( "Test3" ), util::fvector4( 1, 1, 1, 1 ));
	BOOST_CHECK_EQUAL(map2.property( "Test4" ), std::string( "Hallo" ));
	BOOST_CHECK_EQUAL(map2.property( "sub/Test1" ), ( int32_t )1);
	BOOST_CHECK_EQUAL(map2.property( "sub/Test2" ), ( int32_t )2);

	map1.property("test1") = M_PI;
	PropertyMap::PathSet rejected=map1.transfer(map2);
	BOOST_REQUIRE_EQUAL(rejected.size(),1); // there should be one rejected
	BOOST_CHECK_EQUAL(*rejected.begin(),"Test1"); //it should be Test1
	BOOST_CHECK_EQUAL(map1.property("test1"), M_PI); // this should not be overwritten
	BOOST_CHECK_EQUAL(map2.property("test1"), M_2_PI); // and this should still be there

	BOOST_CHECK(map1.transfer(map2,true).empty()); // with overwrite it should be now
	BOOST_CHECK_EQUAL(map1.property("Test1"), M_2_PI); 

	BOOST_CHECK(map2.transfer(map1,"sub").empty()); //subtree
	BOOST_CHECK(!map1.hasBranch("sub")); //subtree no more

	BOOST_CHECK_EQUAL(map2.property( "Test1" ), ( int32_t )1); // its here now
	BOOST_CHECK_EQUAL(map2.property( "Test2" ), ( int32_t )2);
}


}
}
