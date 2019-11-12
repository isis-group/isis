#define BOOST_TEST_MODULE ValueArrayTest
#include <boost/test/unit_test.hpp>

#include <isis/core/tmpfile.hpp>
#include <isis/core/fileptr.hpp>
#define BOOST_FILESYSTEM_VERSION 3 
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

namespace isis
{
namespace test
{

BOOST_AUTO_TEST_CASE( FilePtr_init_test )
{
	util::TmpFile testfile;
	BOOST_REQUIRE_EQUAL( boost::filesystem::file_size( testfile ), 0 );
	{
		data::FilePtr fptr1( testfile, 1024, true ); // create a file for writing
		BOOST_REQUIRE( fptr1.good() ); // it should be "good"
		BOOST_REQUIRE_EQUAL( boost::filesystem::file_size( testfile ), 1024 ); // and it should have the size 1024 by now
		BOOST_CHECK_EQUAL( fptr1.getLength(), 1024 );
	}//fptr1 must be freed / testfile must be closed before trying to open it again or windows will cry for its mamma

	data::FilePtr fptr2( testfile ); // create a file for reading
	BOOST_REQUIRE( fptr2.good() ); // it should be "good"
	BOOST_CHECK_EQUAL( fptr2.getLength(), boost::filesystem::file_size( testfile ) ); // should get the length of the file
}

BOOST_AUTO_TEST_CASE( FilePtr_write_test )
{
	util::TmpFile testfile;
	// creating a new file and write into it
	{
		{
			data::FilePtr fptr( testfile, 1024, true ); // create a file for writing
			BOOST_REQUIRE( fptr.good() ); // it should be "good"
			BOOST_REQUIRE_EQUAL( fptr.getLength(), 1024 );

			data::ValueArray<uint8_t> ptr = fptr.at<uint8_t>( 5 );
			strcpy( ( char * )&ptr[0], "Hello_world!\n" ); // writing to a ValueArray created from a writing fileptr should write into the file
		}
		boost::filesystem::ifstream in( testfile );
		BOOST_REQUIRE( in.good() );

		in.seekg( 5 );
		std::string red;
		in >> red;
		BOOST_CHECK_EQUAL( red, "Hello_world!" );
	}

	// open existing file for reading
	{
		{
			data::FilePtr fptr( testfile ); // create a file for reading
			BOOST_REQUIRE_EQUAL( fptr.getLength(), 1024 );
			BOOST_REQUIRE( fptr.good() );
			data::ValueArray<uint8_t> ptr = fptr.at<uint8_t>( 5 );
			BOOST_CHECK_EQUAL( std::string( ( char * )&ptr[0] ), "Hello_world!\n" ); // reading should get the content of the file
			strcpy( ( char * )&ptr[0], "Hello_you!\n" ); // writing to a ValueArray created from a reading fileptr should _NOT_ trigger a copy-on-write
			BOOST_CHECK_EQUAL( std::string( ( char * )&ptr[0] ), "Hello_you!\n" ); // so next reading should get the new content of the memory
		}
		boost::filesystem::ifstream in( testfile );
		BOOST_REQUIRE( in.good() );

		in.seekg( 5 );
		std::string red;
		in >> red;
		BOOST_CHECK_EQUAL( red, "Hello_world!" ); // but the file behind should not be changed
	}

	// re-use existing file for (over)writing
	{
		{
			data::FilePtr fptr( testfile, 1024, true ); // create a file for writing
			BOOST_REQUIRE( fptr.good() ); // it should be "good"
			BOOST_REQUIRE_EQUAL( fptr.getLength(), 1024 );

			data::ValueArray<uint8_t> ptr = fptr.at<uint8_t>( 5 );
			strcpy( ( char * )&ptr[0], "Hello_universe!\n" ); // writing to a ValueArray created from a writing fileptr should write into the file
		}
		boost::filesystem::ifstream in( testfile );
		BOOST_REQUIRE( in.good() );

		in.seekg( 5 );
		std::string red;
		in >> red;
		BOOST_CHECK_EQUAL( red, "Hello_universe!" );
	}

}

BOOST_AUTO_TEST_CASE( FilePtr_at_test )
{
	util::TmpFile testfile;
	boost::filesystem::ofstream out( testfile );
	util::fvector4 vec1( {1, 2, 3, 4} ), vec2( {5, 6, 7, 8} );

	out.seekp( 5 );
	out.write( ( char * )&vec1[0], sizeof( float ) * 4 );
	out.write( ( char * )&vec2[0], sizeof( float ) * 4 );
	out.close();

	data::FilePtr fptr( testfile ); // create a file for writing
	BOOST_REQUIRE( fptr.good() ); // it should be "good"
	BOOST_REQUIRE_EQUAL( fptr.getLength(), sizeof( float ) * 8 + 5 );

	data::ValueArray<util::fvector4> ptr1 = fptr.at<util::fvector4>( 5 );
	BOOST_CHECK_EQUAL( ptr1[0], util::fvector4( {1, 2, 3, 4} ) );
	BOOST_CHECK_EQUAL( ptr1[1], util::fvector4( {5, 6, 7, 8} ) );

	data::ValueArrayReference ref = fptr.atByID( data::ValueArray<util::fvector4>::staticID(), 5 );
	BOOST_REQUIRE( ref->is<util::fvector4>() );
	data::ValueArray<util::fvector4> ptr2 = ref->castToValueArray<util::fvector4>();
	BOOST_CHECK_EQUAL( ptr2[0], util::fvector4( {1, 2, 3, 4} ) );
	BOOST_CHECK_EQUAL( ptr2[1], util::fvector4( {5, 6, 7, 8} ) );

}

}
}
