/*
 * typePtrTest.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: proeger
 */

#define BOOST_TEST_MODULE TypePtrTest
#include <boost/test/included/unit_test.hpp>
#include "CoreUtils/type.hpp"
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
		if( mesg.str() == "Automatic numeric conversion of {s} to u16bit failed: bad numeric conversion: negative overflow" )
			hit++;
		else
			std::cout << "Unexpected error " << mesg.merge();
	}
};
int TestHandler::hit = 0;

class ReferenceTest: public util::TypePtr<int>::Reference
{
public:
	ReferenceTest(): util::TypePtr<int>::Reference( new util::TypePtr<int>( ( int* )calloc( 5, sizeof( int ) ), 5, Deleter() ) ) {}
	//we have to wrap this into a class, because you cannot directly create a Reference of a pointer
};

bool Deleter::deleted = false;

BOOST_AUTO_TEST_CASE( typePtr_init_test )
{
//  util::enable_log<util::DefaultMsgPrint>(verbose_info);
	BOOST_CHECK( not Deleter::deleted );
	{
		util::TypePtr<int> outer;
		// default constructor must create an empty pointer
		BOOST_CHECK_EQUAL( outer.len(), 0 );
		BOOST_CHECK( not ( boost::shared_ptr<int> )outer );
		BOOST_CHECK_EQUAL( ( ( boost::shared_ptr<int> )outer ).use_count(), 0 );
		{
			util::TypePtr<int> inner( ( int* )calloc( 5, sizeof( int ) ), 5, Deleter() );
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int> &dummy = inner; //get the smart_pointer inside, because TypePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer.len(), inner.len() );
			BOOST_CHECK_EQUAL( ( ( boost::shared_ptr<int> )outer ).get(), ( ( boost::shared_ptr<int> )inner ).get() );
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int> &dummy = outer;
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_clone_test )
{
	Deleter::deleted = false;
	{
		util::_internal::TypePtrBase::Reference outer;
		{
			util::TypePtr<int> inner( ( int* )calloc( 5, sizeof( int ) ), 5, Deleter() );
			// for now we have only one TypePtr referencing the data
			boost::shared_ptr<int> &dummy = inner; //get the smart_pointer inside, because TypePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer->len(), inner.len() );
			BOOST_CHECK_EQUAL(
				( ( boost::shared_ptr<int> )outer->cast_to_TypePtr<int>() ).get(),
				( ( boost::shared_ptr<int> )inner ).get()
			);
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int> &dummy = outer->cast_to_TypePtr<int>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_Reference_test )
{
	Deleter::deleted = false;
	{
		util::TypePtr<int>::Reference outer;
		// default constructor must create an empty pointer-ref
		BOOST_CHECK( outer.empty() );
		{
			ReferenceTest inner;
			BOOST_CHECK( not inner.empty() );
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int> &dummy1 = inner->cast_to_TypePtr<int>();//get the smart_pointer inside the referenced TypePtr
			BOOST_CHECK_EQUAL( dummy1.use_count(), 1 ); //We only have one TypePtr (inside inner)
			outer = inner;//now we have two
			boost::shared_ptr<int> &dummy2 = outer->cast_to_TypePtr<int>();
			BOOST_CHECK_EQUAL( dummy1.use_count(), 2 );
			BOOST_CHECK_EQUAL( dummy2.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( dummy1.get(), dummy2.get() );
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int> &dummy = outer->cast_to_TypePtr<int>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_splice_test )
{
	Deleter::deleted = false;
	{
		std::vector<util::_internal::TypePtrBase::Reference> outer;
		{
			util::TypePtr<int> inner( ( int* )calloc( 5, sizeof( int ) ), 5, Deleter() );
			// for now we have only one pointer referencing the data
			boost::shared_ptr<int> &dummy = inner; //get the smart_pointer inside, because TypePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			//splicing up makes a references for every splice
			outer = inner.splice( 2 );
			BOOST_CHECK_EQUAL( outer.size(), 5 / 2 + 1 );// 5/2 normal splices plus one halve  splice
			//the shall be outer.size() references from the splices, plus one for the origin
			BOOST_CHECK_EQUAL( dummy.use_count(), outer.size() + 1 );
		}
		boost::shared_ptr<int> &dummy = outer.front()->cast_to_TypePtr<int>();
		assert( boost::get_deleter<util::TypePtr<int>::DelProxy>( dummy ) );
		BOOST_CHECK_EQUAL( outer.front()->len(), 2 );// the first slices shall be of the size 2
		BOOST_CHECK_EQUAL( outer.back()->len(), 1 );// the last slice shall be of the size 1 (5%2)
		//we cannot ask for the use_count of the original because its hidden in DelProxy (outer[0].use_count will get the use_count of the splice)
		//but we can check if it was allready deleted (it shouldn't, because the splices are still using that data)
		BOOST_CHECK( not Deleter::deleted );
	}
	//now that all splices are gone the original data shall be deleted
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( typePtr_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	util::TypePtr<float> floatArray( ( float* )malloc( sizeof( float ) * 12 ), 12 );
	//without scaling
	floatArray.copyFromMem( init, 12 );

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( floatArray[i], init[i] );

	util::TypePtr<int> intArray = floatArray.copyToNew<int>();

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( intArray[i], round( init[i] ) );

	//with scaling
	const double scale = std::min( std::numeric_limits< short >::max() / 2e5, std::numeric_limits< short >::min() / -2e5 );

	for ( int i = 0; i < 12; i++ )
		floatArray[i] = init[i] * 1e5;

	util::TypePtr<short> shortArray = floatArray.copyToNew<short>();

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( shortArray[i], round( init[i] * 1e5 * scale ) );

	//with offset and scale
	const double uscale = std::numeric_limits< unsigned short >::max() / 4e5;
	util::TypePtr<unsigned short> ushortArray = floatArray.copyToNew<unsigned short>();

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( ushortArray[i], round( init[i] * 1e5 * uscale + 32767.5 ) );
}

BOOST_AUTO_TEST_CASE( typePtr_minmax_test )
{
	const float init[] = { -1.8, -1.5, -1.3, -0.6, -0.2, 1.8, 1.5, 1.3, 0.6, 0.2};
	util::TypePtr<float> floatArray( ( float* )malloc( sizeof( float ) * 10 ), 10 );
	//without scaling
	floatArray.copyFromMem( init, 10 );
	{
		util::Type<float> min, max;
		floatArray.getMinMax( min, max );
		BOOST_CHECK_EQUAL( min, -1.8f );
		BOOST_CHECK_EQUAL( max, 1.8f );
	}
	{
		util::Type<int> min, max;
		floatArray.getMinMax( min, max );
		BOOST_CHECK_EQUAL( min, -2 );
		BOOST_CHECK_EQUAL( max, 2 );
	}
	{
		util::enable_log<TestHandler>( error );
		util::Type<u_int16_t> min, max;
		floatArray.getMinMax( min, max );
		BOOST_CHECK_EQUAL( TestHandler::hit, 4 );
	}
}
}
}
