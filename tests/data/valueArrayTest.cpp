/*
 * ValueArrayTest.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: proeger
 */

#define NOMINMAX 1
#define BOOST_TEST_MODULE ValueArrayTest
#include <boost/test/unit_test.hpp>
#include <isis/data/valuearray.hpp>
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
class TestHandler : public util::MessageHandlerBase
{
public:
	static int hit;
	TestHandler( LogLevel level ): util::MessageHandlerBase( level ) {}
	virtual ~TestHandler() {}
	void commit( const util::Message &mesg ) {
		if ( mesg.str() == "Automatic numeric conversion of {s} to u16bit failed: bad numeric conversion: negative overflow" )
			hit++;
		else
			std::cout << "Unexpected error " << mesg.merge("");
	}
};
int TestHandler::hit = 0;

class ReferenceTest: public data::ValueArray<int32_t>::Reference
{
public:
	ReferenceTest(): data::ValueArray<int32_t>::Reference( new data::ValueArray<int32_t>( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() ) ) {}
	//we have to wrap this into a class, because you cannot directly create a Reference of a pointer
};

bool Deleter::deleted = false;

BOOST_AUTO_TEST_CASE( ValueArray_init_test )
{
	BOOST_CHECK( ! Deleter::deleted );
	{
		data::enableLog<util::DefaultMsgPrint>( error );
		data::ValueArray<int32_t> outer( 0 );
		data::enableLog<util::DefaultMsgPrint>( warning );
		// must create an empty pointer
		BOOST_CHECK_EQUAL( outer.getLength(), 0 );
		BOOST_CHECK( ! ( std::shared_ptr<int32_t> )outer );
		BOOST_CHECK_EQUAL( ( ( std::shared_ptr<int32_t> )outer ).use_count(), 0 );
		{
			data::ValueArray<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one pointer referencing the data
			std::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValueArray does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer.getLength(), inner.getLength() );
			BOOST_CHECK_EQUAL( ( ( std::shared_ptr<int32_t> )outer ).get(), ( ( std::shared_ptr<int32_t> )inner ).get() );
		}
		//now again its only one (inner is gone)
		std::shared_ptr<int32_t> &dummy = outer;
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValueArray_clone_test )
{
	Deleter::deleted = false;
	{
		data::ValueArrayReference outer;
		{
			data::ValueArray<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one ValueArray referencing the data
			std::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValueArray does not have/need use_count
			BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
			outer = inner;//now we have two
			BOOST_CHECK_EQUAL( dummy.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( outer->getLength(), inner.getLength() );
			BOOST_CHECK_EQUAL(
				( ( std::shared_ptr<int32_t> )outer->castToValueArray<int32_t>() ).get(),
				( ( std::shared_ptr<int32_t> )inner ).get()
			);
		}
		//now again its only one (inner is gone)
		std::shared_ptr<int32_t> &dummy = outer->castToValueArray<int32_t>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValueArray_Reference_test )
{
	Deleter::deleted = false;
	{
		data::ValueArray<int32_t>::Reference outer;
		// default constructor must create an empty pointer-ref
		BOOST_CHECK( outer.isEmpty() );
		{
			ReferenceTest inner;
			BOOST_CHECK( ! inner.isEmpty() );
			// for now we have only one pointer referencing the data
			std::shared_ptr<int32_t> &dummy1 = inner->castToValueArray<int32_t>();//get the smart_pointer inside the referenced ValueArray
			BOOST_CHECK_EQUAL( dummy1.use_count(), 1 ); //We only have one ValueArray (inside inner)
			outer = inner;//now we have two
			std::shared_ptr<int32_t> &dummy2 = outer->castToValueArray<int32_t>();
			BOOST_CHECK_EQUAL( dummy1.use_count(), 2 );
			BOOST_CHECK_EQUAL( dummy2.use_count(), 2 );
			//and both reference the same data
			BOOST_CHECK_EQUAL( dummy1.get(), dummy2.get() );
		}
		//now again its only one (inner is gone)
		std::shared_ptr<int32_t> &dummy = outer->castToValueArray<int32_t>();
		BOOST_CHECK_EQUAL( dummy.use_count(), 1 );
	}
	//data should be deleted by now (outer is gone)
	BOOST_CHECK( Deleter::deleted );
}

BOOST_AUTO_TEST_CASE( ValueArray_splice_test )
{
	Deleter::deleted = false;
	{
		std::vector<data::ValueArrayReference> outer;
		{
			data::ValueArray<int32_t> inner( ( int32_t * )calloc( 5, sizeof( int32_t ) ), 5, Deleter() );
			// for now we have only one pointer referencing the data
			std::shared_ptr<int32_t> &dummy = inner; //get the smart_pointer inside, because ValueArray does not have/need use_count
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

BOOST_AUTO_TEST_CASE( ValueArray_conv_scaling_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValueArray<float> floatArray( 12 );

	//scaling to itself should allways be 1/0
	floatArray.copyFromMem( init, 12 );
	data::scaling_pair scale = floatArray.getScalingTo( data::ValueArray<float>::staticID() );
	BOOST_CHECK_EQUAL( scale.first->as<float>(), 1 );
	BOOST_CHECK_EQUAL( scale.second->as<float>(), 0 );

	//float=> integer should upscale
	scale = floatArray.getScalingTo( data::ValueArray<int32_t>::staticID() );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), std::numeric_limits<int32_t>::max() / 2. );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 0 );

	//float=> unsigned integer should upscale and shift
	scale = floatArray.getScalingTo( data::ValueArray<uint8_t>::staticID() );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), std::numeric_limits<uint8_t>::max() / 4. );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 2 * scale.first->as<double>() );
}

BOOST_AUTO_TEST_CASE( ValueArray_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValueArray<float> floatArray( 12 );
	//with automatic upscaling into integer
	floatArray.copyFromMem( init, 12 );

	for ( int i = 0; i < 12; i++ )
		BOOST_REQUIRE_EQUAL( floatArray[i], init[i] );

	data::ValueArray<int32_t> intArray = floatArray.copyAs<int32_t>();
	double scale = std::numeric_limits<int32_t>::max() / 2.;

	for ( int i = 0; i < 12; i++ ) // Conversion from float to integer will scale up to maximum, to map the fraction as exact as possible
		BOOST_CHECK_EQUAL( intArray[i], ceil( init[i]*scale - .5 ) );

	//with automatic downscaling because of range overflow
	scale = std::min( std::numeric_limits< short >::max() / 2e5, std::numeric_limits< short >::min() / -2e5 );

	for ( int i = 0; i < 12; i++ )
		floatArray[i] = init[i] * 1e5;

	data::enableLog<util::DefaultMsgPrint>( error );
	data::ValueArray<short> shortArray = floatArray.copyAs<short>();
	data::ValueArray<uint8_t> byteArray = shortArray.copyAs<uint8_t>();
	data::enableLog<util::DefaultMsgPrint>( warning );

	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( shortArray[i], ceil( init[i] * 1e5 * scale - .5 ) );

	//with offset and scale
	const double uscale = std::numeric_limits< unsigned short >::max() / 4e5;
	data::enableLog<util::DefaultMsgPrint>( error );
	data::ValueArray<unsigned short> ushortArray = floatArray.copyAs<unsigned short>();
	data::enableLog<util::DefaultMsgPrint>( warning );


	for ( int i = 0; i < 12; i++ )
		BOOST_CHECK_EQUAL( ushortArray[i], ceil( init[i] * 1e5 * uscale + 32767.5 - .5 ) );
}

BOOST_AUTO_TEST_CASE( ValueArray_complex_minmax_test )
{
	const std::complex<float> init[] = { std::complex<float>( -2, 1 ), -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, std::complex<float>( 0.2, -5 )};
	data::ValueArray<std::complex<float> > cfArray( 12 );
	cfArray.copyFromMem( init, 12 );
	float minMag=  std::numeric_limits<float>::max(),maxMag=  -std::numeric_limits<float>::max();
	float minPhase=std::numeric_limits<float>::max(),maxPhase=-std::numeric_limits<float>::max();
	for(std::complex<float> v:cfArray){
		const float mag=std::abs(v), phase=std::arg(v);
		if(minMag>mag)minMag=mag;
		if(maxMag<mag)maxMag=mag;
		if(minPhase>phase)minPhase=phase;
		if(maxPhase<phase)maxPhase=phase;
	}
	std::pair< util::ValueReference, util::ValueReference > minmax = cfArray.getMinMax();

	BOOST_CHECK_EQUAL( minmax.first->as<std::complex<float> >(),  std::polar( minMag, minPhase ) );
	BOOST_CHECK_EQUAL( minmax.second->as<std::complex<float> >(), std::polar( maxMag, maxPhase ) );
}

BOOST_AUTO_TEST_CASE( ValueArray_complex_conversion_test )
{
	const std::complex<float> init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValueArray<std::complex<float> > cfArray( 12 );
	cfArray.copyFromMem( init, 12 );
	data::ValueArray<std::complex<double> > cdArray = cfArray.copyAs<std::complex<double> >();

	for( size_t i = 0; i < 12; i++ ) {
		BOOST_CHECK_EQUAL( cfArray[i], init[i] );
		BOOST_CHECK_EQUAL( cdArray[i], std::complex<double>( init[i] ) );
	}
}
BOOST_AUTO_TEST_CASE( ValueArray_numeric_to_complex_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, -0.2, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValueArray<float> fArray( sizeof( init ) / sizeof( float ) );
	fArray.copyFromMem( init, sizeof( init ) / sizeof( float ) );

	data::ValueArray<std::complex<float> > cfArray = fArray.copyAs<std::complex<float> >();
	data::ValueArray<std::complex<double> > cdArray = fArray.copyAs<std::complex<double> >();

	// scalar values should be in real, imag should be 0
	for( size_t i = 0; i < sizeof( init ) / sizeof( float ); i++ ) {
		BOOST_CHECK_EQUAL( cfArray[i].real(), init[i] );
		BOOST_CHECK_EQUAL( cdArray[i].real(), init[i] );
		BOOST_CHECK_EQUAL( cfArray[i].imag(), 0 );
		BOOST_CHECK_EQUAL( cdArray[i].imag(), 0 );
	}

}


BOOST_AUTO_TEST_CASE( ValueArray_color_minmax_test )
{

	const util::color48 init[] = {
		{ 20, 180, 150},
		{130,  60,  20},
		{ 20, 180, 150},
		{130,  60,  20}
	};
	data::ValueArray<util::color48> ccArray( 4 );
	ccArray.copyFromMem( init, 4 );
	std::pair< util::ValueReference, util::ValueReference > minmax = ccArray.getMinMax();
	const util::color48 colmin = {20, 60, 20}, colmax = {130, 180, 150};

	BOOST_CHECK_EQUAL( minmax.first->as<util::color48>(), colmin );
	BOOST_CHECK_EQUAL( minmax.second->as<util::color48>(), colmax );
}

BOOST_AUTO_TEST_CASE( ValueArray_color_conversion_test )
{
	const util::color48 init[] = { {0, 0, 0}, {100, 2, 4}, {200, 200, 200}, {510, 4, 2}};
	data::ValueArray<util::color48 > c16Array( 4 );
	c16Array.copyFromMem( init, 4 );

	data::ValueArray<util::color24 > c8Array = c16Array.copyAs<util::color24 >();// should downscale (by 2)

	for( size_t i = 0; i < 4; i++ ) {
		BOOST_REQUIRE_EQUAL( c16Array[i], init[i] );
		BOOST_CHECK_EQUAL( c8Array[i].r, init[i].r / 2 );
		BOOST_CHECK_EQUAL( c8Array[i].g, init[i].g / 2 );
		BOOST_CHECK_EQUAL( c8Array[i].b, init[i].b / 2 );
	}

	c16Array = c8Array.copyAs<util::color48>();//should not scale

	for( size_t i = 0; i < 4; i++ ) {
		BOOST_CHECK_EQUAL( c16Array[i], c8Array[i] );
	}

}


BOOST_AUTO_TEST_CASE( ValueArray_numeric_to_color_conversion_test )
{
	const short init[] = {0, 0, 0, 100, 2, 4, 200, 200, 200, 510, 4, 2};
	data::ValueArray<uint16_t> i16Array( sizeof( init ) / sizeof( uint16_t ) );
	i16Array.copyFromMem( init, sizeof( init ) / sizeof( uint16_t ) );

	//scaling should be 0.5/0
	data::scaling_pair scale = i16Array.getScalingTo( data::ValueArray<util::color24 >::staticID() );
	BOOST_CHECK_EQUAL( scale.first->as<double>(), 0.5 );
	BOOST_CHECK_EQUAL( scale.second->as<double>(), 0 );

	data::ValueArray<util::color24 > c8Array = i16Array.copyAs<util::color24 >();// should downscale (by 2)

	for( size_t i = 0; i < sizeof( init ) / sizeof( uint16_t ); i++ ) {
		BOOST_CHECK_EQUAL( c8Array[i].r, init[i] / 2 );
		BOOST_CHECK_EQUAL( c8Array[i].g, init[i] / 2 );
		BOOST_CHECK_EQUAL( c8Array[i].b, init[i] / 2 );
	}

	data::ValueArray<util::color48 > c16Array = i16Array.copyAs<util::color48 >(); //should not scale

	for( size_t i = 0; i < sizeof( init ) / sizeof( uint16_t ); i++ ) {
		BOOST_CHECK_EQUAL( c16Array[i].r, init[i] );
		BOOST_CHECK_EQUAL( c16Array[i].g, init[i] );
		BOOST_CHECK_EQUAL( c16Array[i].b, init[i] );
	}
}

BOOST_AUTO_TEST_CASE( ValueArray_boolean_conversion_test )
{
	const float init[] = { -2, -1.8, -1.5, -1.3, -0.6, 0, 2, 1.8, 1.5, 1.3, 0.6, 0.2};
	data::ValueArray<float> cfArray( 12 );
	cfArray.copyFromMem( init, 12 );
	data::ValueArray<bool> bArray = cfArray.copyAs<bool>();
	data::ValueArray<float> fArray = bArray.copyAs<float>();

	for( size_t i = 0; i < 12; i++ ) {
		BOOST_CHECK_EQUAL( bArray[i], ( bool )init[i] );
		BOOST_CHECK_EQUAL( fArray[i], ( init[i] ? 1 : 0 ) );
	}
}

BOOST_AUTO_TEST_CASE( ValueArray_minmax_test )
{
	const float init[] = {
		-std::numeric_limits<float>::infinity(),
		-1.8, -1.5, -1.3, -0.6, -0.2, 1.8, 1.5, 1.3,
		static_cast<float>( sqrt( -1 ) ),
		std::numeric_limits<float>::infinity(), 0.6, 0.2
	};
	data::ValueArray<float> floatArray( sizeof( init ) / sizeof( float ) );
	//without scaling
	floatArray.copyFromMem( init, sizeof( init ) / sizeof( float ) );
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
	data::ValueArray<T> array( 1024 );
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
BOOST_AUTO_TEST_CASE( ValueArray_rnd_minmax_test )
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


BOOST_AUTO_TEST_CASE( ValueArray_iterator_test )
{
	data::ValueArray<short> array( 1024 );

	for( int i = 0; i < 1024; i++ )
		array[i] = i + 1;

	size_t cnt = 0;

	for( data::ValueArray< short >::iterator i = array.begin(); i != array.end(); i++, cnt++ ) {
		BOOST_CHECK_EQUAL( *i, cnt + 1 ); // normal increment
		BOOST_CHECK_EQUAL( *( array.begin() + cnt ), cnt + 1 ); //+=
	}

	cnt=0;
	for( auto x: array ) {
		BOOST_CHECK_EQUAL( x, ++cnt ); // normal increment
	}


	BOOST_CHECK_EQUAL( *std::min_element( array.begin(), array.end() ), 1 );
	BOOST_CHECK_EQUAL( *std::max_element( array.begin(), array.end() ), *( array.end() - 1 ) );

	BOOST_CHECK_EQUAL( std::distance( array.begin(), array.end() ), 1024 );

	BOOST_CHECK_EQUAL( std::accumulate( array.begin(), array.end(), 0 ), 1024 * ( 1024 + 1 ) / 2 ); //gauss is my homie :-D

	std::fill( array.begin(), array.end(), 0 );
	BOOST_CHECK_EQUAL( std::accumulate( array.begin(), array.end(), 0 ), 0 );
}
BOOST_AUTO_TEST_CASE( ValueArray_generic_iterator_test )
{
	data::ValueArray<short> array( 1024 );
	data::ValueArrayBase &generic_array = array;

	// filling, using generic access
	for( int i = 0; i < 1024; i++ )
		generic_array.beginGeneric()[i] = util::Value<int>( i + 1 ); //(miss)using the iterator for indexed access

	// check the content
	size_t cnt = 0;

	for( data::ValueArrayBase::value_iterator i = generic_array.beginGeneric(); i != generic_array.endGeneric(); i++, cnt++ ) {
		BOOST_CHECK_EQUAL( *i, util::Value<int>( cnt + 1 ) ); //this is using ValueBase::eq
	}

	//check searching operations
	BOOST_CHECK_EQUAL( *std::min_element( generic_array.beginGeneric(), generic_array.endGeneric() ), *generic_array.beginGeneric() );
	BOOST_CHECK_EQUAL( *std::max_element( generic_array.beginGeneric(), generic_array.endGeneric() ), *( generic_array.endGeneric() - 1 ) );
	BOOST_CHECK( std::find( generic_array.beginGeneric(), generic_array.endGeneric(), util::Value<int>( 5 ) ) == generic_array.beginGeneric() + 4 ); //"1+4"

	BOOST_CHECK_EQUAL( std::distance( array.begin(), array.end() ), 1024 );
}
}
}
