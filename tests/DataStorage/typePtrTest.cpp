/*
 * ValuePtrTest.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: proeger
 */

#define NOMINMAX 1
#define BOOST_TEST_MODULE ValuePtrTest
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <DataStorage/typeptr.hpp>
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

BOOST_AUTO_TEST_CASE( ValuePtr_init_test )
{
	BOOST_CHECK( ! Deleter::deleted );
	{
		data::enableLog<util::DefaultMsgPrint>(error);
		data::ValuePtr<int32_t> outer( 0 );
		data::enableLog<util::DefaultMsgPrint>(warning);
		// must create an empty pointer
		BOOST_CHECK_EQUAL( outer.getLength(), 0 );
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
			BOOST_CHECK_EQUAL( outer.getLength(), inner.getLength() );
			BOOST_CHECK_EQUAL( ( ( boost::shared_ptr<int32_t> )outer ).get(), ( ( boost::shared_ptr<int32_t> )inner ).get() );
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int32_t> &dummy = outer;
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValuePtr_clone_test )
{
	Deleter::deleted = false;
	{
		data::ValuePtrReference outer;
		{
			data::ValuePtr<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one ValuePtr referencing the data
			boost::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValuePtr does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer->getLength(), inner.getLength() );
			BOOST_CHECK_EQUAL(
				( ( boost::shared_ptr<int32_t> )outer->castToValuePtr<int32_t>() ).get(),
				( ( boost::shared_ptr<int32_t> )inner ).get()
			);
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int32_t> &dummy = outer->castToValuePtr<int32_t>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValuePtr_Reference_test )
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
			boost::shared_ptr<int32_t> &dummy1 = inner->castToValuePtr<int32_t>();//get the smart_pointer inside the referenced ValuePtr
			BOOST_CHECK_EQUAL( dummy1.use_count(), 1 ); //We only have one ValuePtr (inside inner)
			outer = inner;//now we have two
			boost::shared_ptr<int32_t> &dummy2 = outer->castToValuePtr<int32_t>();
			BOOST_CHECK_EQUAL( dummy1.use_count(), 2 );
			BOOST_CHECK_EQUAL( dummy2.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( dummy1.get(), dummy2.get() );
		}
		//now again its only one (inner is gone)
		boost::shared_ptr<int32_t> &dummy = outer->castToValuePtr<int32_t>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValuePtr_splice_test )
{
	Deleter::deleted = false;
	{
		std::vector<data::ValuePtrReference> outer;
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
		BOOST_CHECK_EQUAL( outer.front()->getLength(), 2 );// the first slices shall be of the size 2
		BOOST_CHECK_EQUAL( outer.back()->getLength(), 1 );// the last slice shall be of the size 1 (5%2)
		//we cannot ask for the use_count of the original because its hidden in DelProxy (outer[0].use_count will get the use_count of the splice)
		//but we can check if it was allready deleted (it shouldn't, because the splices are still using that data)
		BOOST_CHECK( ! Deleter::deleted );
	}
	//now that all splices are gone the original data shall be deleted
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValuePtr_conv_scaling_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> floatArray( 12 );

	//scaling to itself should allways be 1/0
	floatArray.copyFromMem( init, 12 );
	data::scaling_pair scale = floatArray.getScalingTo( data::ValuePtr<float>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<float>(), 1 );
	BOOST_CHECK_EQUAL( scale.second->as<float>(), 0 );

	//float=> integer should upscale
	scale = floatArray.getScalingTo( data::ValuePtr<int32_t>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), std::numeric_limits<int32_t>::max() / 2. );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 0 );

	//float=> unsigned integer should upscale and shift
	scale = floatArray.getScalingTo( data::ValuePtr<uint8_t>::staticID );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), std::numeric_limits<uint8_t>::max() / 4. );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 2 * scale.first->as<double>() );
}

BOOST_AUTO_TEST_CASE( ValuePtr_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> floatArray( 12 );
	//with automatic upscaling into integer
	floatArray.copyFromMem( init, 12 );

	for ( int i = 0; i < 12; i++ )
		BOOST_REQUIRE_EQUAL( floatArray[i], init[i] );

	data::ValuePtr<int32_t> intArray = floatArray.copyAs<int32_t>();
	double scale = std::numeric_limits<int32_t>::max() / 2.;

	for ( int i = 0; i < 12; i++ ) // Conversion from float to integer will scale up to maximum, to map the fraction as exact as possible
		BOOST_CHECK_EQUAL( intArray[i], ceil( init[i]*scale - .5 ) );

	//with automatic downscaling because of range overflow
	scale = std::min( std::numeric_limits< short >::max() / 2e5, std::numeric_limits< short >::min() / -2e5 );

	for ( int i = 0; i < 12; i++ )
		floatArray[i] = init[i] * 1e5;

	data::enableLog<util::DefaultMsgPrint>(error);
	data::ValuePtr<short> shortArray = floatArray.copyAs<short>();
	data::ValuePtr<uint8_t> byteArray = shortArray.copyAs<uint8_t>();
	data::enableLog<util::DefaultMsgPrint>(warning);

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( shortArray[i], ceil( init[i] * 1e5 * scale - .5 ) );

	//with offset and scale
	const double uscale = std::numeric_limits< unsigned short >::max() / 4e5;
	data::enableLog<util::DefaultMsgPrint>(error);
	data::ValuePtr<unsigned short> ushortArray = floatArray.copyAs<unsigned short>();
	data::enableLog<util::DefaultMsgPrint>(warning);


	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( ushortArray[i], ceil( init[i] * 1e5 * uscale + 32767.5 - .5 ) );
}

BOOST_AUTO_TEST_CASE( ValuePtr_complex_conversion_test )
{
	const std::complex<float> init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<std::complex<float> > cfArray( 12 );
	cfArray.copyFromMem( init, 12 );
	data::ValuePtr<std::complex<double> > cdArray = cfArray.copyAs<std::complex<double> >();

	for( size_t i = 0; i < 12; i++ ) {
		BOOST_CHECK_EQUAL( cfArray[i], init[i] );
		BOOST_CHECK_EQUAL( cdArray[i], std::complex<double>( init[i] ) );
	}
}

BOOST_AUTO_TEST_CASE( ValuePtr_boolean_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, 0, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> cfArray( 12 );
	cfArray.copyFromMem( init, 12 );
	data::ValuePtr<bool> bArray = cfArray.copyAs<bool>();
	data::ValuePtr<float> fArray = bArray.copyAs<float>();

	for( size_t i = 0; i < 12; i++ ) {
		BOOST_CHECK_EQUAL( bArray[i], ( bool )init[i] );
		BOOST_CHECK_EQUAL( fArray[i], ( init[i] ? 1 : 0 ) );
	}
}

BOOST_AUTO_TEST_CASE( ValuePtr_minmax_test )
{
	const float init[] = { -1.8, -1.5, -1.3, -0.6, -0.2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValuePtr<float> floatArray( 10 );
	//without scaling
	floatArray.copyFromMem( init, 10 );
	{
		std::pair<util::ValueReference, util::ValueReference> minmax = floatArray.getMinMax();
		BOOST_CHECK( minmax.first->is<float>() );
		BOOST_CHECK( minmax.second->is<float>() );
		BOOST_CHECK_EQUAL( minmax.first->as<float>(), -1.8f );
		BOOST_CHECK_EQUAL( minmax.second->as<float>(), 1.8f );
	}
}

template<typename T> void minMaxInt()
{
	data::ValuePtr<T> array( 1024 );
	double div = static_cast<double>( RAND_MAX ) / std::numeric_limits<T>::max();

	for( int i = 0; i < 1024; i++ )
		array[i] = rand() / div;

	array[40] = std::numeric_limits<T>::max();
	array[42] = std::numeric_limits<T>::min();

	std::pair<util::ValueReference, util::ValueReference> minmax = array.getMinMax();
	BOOST_CHECK( minmax.first->is<T>() );
	BOOST_CHECK( minmax.second->is<T>() );
	BOOST_CHECK_EQUAL( minmax.first->as<T>(), std::numeric_limits<T>::min() );
	BOOST_CHECK_EQUAL( minmax.second->as<T>(), std::numeric_limits<T>::max() );
}
BOOST_AUTO_TEST_CASE( ValuePtr_rnd_minmax_test )
{
	minMaxInt< uint8_t>();
	minMaxInt<uint16_t>();
	minMaxInt<uint32_t>();

	minMaxInt< int8_t>();
	minMaxInt<int16_t>();
	minMaxInt<int32_t>();

	minMaxInt< float>();
	minMaxInt<double>();
}

}
}
