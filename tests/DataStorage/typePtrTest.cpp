/*
 * typePtrTest.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: proeger
 */

#define NOMINMAX 1
#define BOOST_TEST_MODULE TypePtrTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "DataStorage/typeptr.hpp"
#include <cmath>


namespace isis
{
namespace test
{

struct Deleter {
	static bool deleted;
	void operator()( void *p ) {
		free( p );
		deleted = true;
	};
};

// Handlers must not be local classes
class TestHandler : public util::_internal::MessageHandlerBase
{
public:
	static int hit;
	TestHandler( LogLevel level ): util::_internal::MessageHandlerBase( level ) {}
	virtual ~TestHandler() {}
	void commit( const util::_internal::Message &mesg ) {
		if ( mesg.str() == "Automatic numeric conversion of {s} to u16bit failed: bad numeric conversion: negative overflow" )
			hit++;
		else
			std::cout << "Unexpected error " << mesg.merge();
	}
};
int TestHandler::hit = 0;

class ReferenceTest: public data::ValuePtr<int32_t>::Reference
{
public:
	ReferenceTest(): data::ValuePtr<int32_t>::Reference( new data::ValuePtr<int32_t>( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() ) ) {}
	//we have to wrap this into a class, because you cannot directly create a Reference of a pointer
};

bool Deleter::deleted = false;

BOOST_AUTO_TEST_CASE( typePtr_init_test )
{
	BOOST_CHECK( ! Deleter::deleted );
	{
		data::ValuePtr<int32_t> outer;
		// default constructor must create an empty pointer
		BOOST_CHECK_EQUAL( outer.length(), 0 );
		BOOST_CHECK( ! ( boost::shared_ptr<int32_t> )outer );
		BOOST_CHECK_EQUAL( ( ( boost::shared_ptr<int32_t> )outer ).use_count(), 0 );
		{
			data::ValuePtr<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValuePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer.length(), inner.length() );
			BOOST_CHECK_EQUAL( ( ( boost::shared_ptr<int32_t> )outer ).get(), ( ( boost::shared_ptr<int32_t> )inner ).get() );
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int32_t> &dummy = outer;
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_clone_test )
{
	Deleter::deleted = false;
	{
		data::TypePtrReference outer;
		{
			data::ValuePtr<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one ValuePtr referencing the data
			boost::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValuePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer->length(), inner.length() );
			BOOST_CHECK_EQUAL(
				( ( boost::shared_ptr<int32_t> )outer->castToTypePtr<int32_t>() ).get(),
				( ( boost::shared_ptr<int32_t> )inner ).get()
			);
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int32_t> &dummy = outer->castToTypePtr<int32_t>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_Reference_test )
{
	Deleter::deleted = false;
	{
		data::ValuePtr<int32_t>::Reference outer;
		// default constructor must create an empty pointer-ref
		BOOST_CHECK( outer.isEmpty() );
		{
			ReferenceTest inner;
			BOOST_CHECK( ! inner.isEmpty() );
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int32_t> &dummy1 = inner->castToTypePtr<int32_t>();//get the smart_pointer inside the referenced ValuePtr
			BOOST_CHECK_EQUAL( dummy1.use_count(), 1 ); //We only have one ValuePtr (inside inner)
			outer = inner;//now we have two
			boost::shared_ptr<int32_t> &dummy2 = outer->castToTypePtr<int32_t>();
			BOOST_CHECK_EQUAL( dummy1.use_count(), 2 );
			BOOST_CHECK_EQUAL( dummy2.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( dummy1.get(), dummy2.get() );
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int32_t> &dummy = outer->castToTypePtr<int32_t>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_splice_test )
{
	Deleter::deleted = false;
	{
		std::vector<data::TypePtrReference> outer;
		{
			data::ValuePtr<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValuePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			//splicing up makes a references for every splice
			outer = inner.splice( 2 );
			BOOST_CHECK_EQUAL( outer.size(), 5 / 2 + 1 );// 5/2 normal splices plus one halve  splice
			//the shall be outer.size() references from the splices, plus one for the origin
			BOOST_CHECK_EQUAL( dummy.use_count(), outer.size() + 1 );
		}
		boost::shared_ptr<int32_t> &dummy = outer.front()->castToTypePtr<int32_t>();

		BOOST_CHECK_EQUAL( outer.front()->length(), 2 );// the first slices shall be of the size 2
		BOOST_CHECK_EQUAL( outer.back()->length(), 1 );// the last slice shall be of the size 1 (5%2)
		//we cannot ask for the use_count of the original because its hidden in DelProxy (outer[0].use_count will get the use_count of the splice)
		//but we can check if it was allready deleted (it shouldn't, because the splices are still using that data)
		BOOST_CHECK( ! Deleter::deleted );
	}
	//now that all splices are gone the original data shall be deleted
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_conv_scaling_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> floatArray( ( float * )malloc( sizeof( float ) * 12 ), 12 );
	//with automatic upscaling into integer
	floatArray.copyFromMem( init, 12 );
	data::scaling_pair scale = floatArray.getScalingTo( data::ValuePtr<float>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<float>(), 1 );
	BOOST_CHECK_EQUAL( scale.second->as<float>(), 0 );

	scale = floatArray.getScalingTo( data::ValuePtr<int32_t>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), std::numeric_limits<int32_t>::max() / 2. );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 0 );
}

BOOST_AUTO_TEST_CASE( typePtr_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> floatArray( ( float * )malloc( sizeof( float ) * 12 ), 12 );
	//with automatic upscaling into integer
	floatArray.copyFromMem( init, 12 );

	for ( int i = 0; i < 12; i++ )
		BOOST_REQUIRE_EQUAL( floatArray[i], init[i] );

	data::ValuePtr<int32_t> intArray = floatArray.copyToNew<int32_t>();
	double scale = std::numeric_limits<int32_t>::max() / 2.;

	for ( int i = 0; i < 12; i++ ) // Conversion from float to integer will scale up to maximum, to map the fraction as exact as possible
		BOOST_CHECK_EQUAL( intArray[i], ceil( init[i]*scale - .5 ) );

	//with automatic downscaling because of range overflow
	scale = std::min( std::numeric_limits< short >::max() / 2e5, std::numeric_limits< short >::min() / -2e5 );

	for ( int i = 0; i < 12; i++ )
		floatArray[i] = init[i] * 1e5;

	data::ValuePtr<short> shortArray = floatArray.copyToNew<short>();
	data::ValuePtr<uint8_t> byteArray = shortArray.copyToNew<uint8_t>();

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( shortArray[i], ceil( init[i] * 1e5 * scale - .5 ) );

	//with offset and scale
	const double uscale = std::numeric_limits< unsigned short >::max() / 4e5;
	data::ValuePtr<unsigned short> ushortArray = floatArray.copyToNew<unsigned short>();

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( ushortArray[i], ceil( init[i] * 1e5 * uscale + 32767.5 - .5 ) );
}

BOOST_AUTO_TEST_CASE( typePtr_minmax_test )
{
	const float init[] = { -1.8, -1.5, -1.3, -0.6, -0.2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> floatArray( ( float * )malloc( sizeof( float ) * 10 ), 10 );
	//without scaling
	floatArray.copyFromMem( init, 10 );
	{
		std::pair<util::TypeReference, util::TypeReference> minmax = floatArray.getMinMax();
		BOOST_CHECK( minmax.first->is<float>() );
		BOOST_CHECK( minmax.second->is<float>() );
		BOOST_CHECK_EQUAL( minmax.first->as<float>(), -1.8f );
		BOOST_CHECK_EQUAL( minmax.second->as<float>(), 1.8f );
	}
}
}
}
