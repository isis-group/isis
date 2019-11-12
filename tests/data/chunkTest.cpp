/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ChunkTest
#include <boost/test/unit_test.hpp>
#include <isis/core/chunk.hpp>

namespace isis
{
namespace test
{

/* create an image */
BOOST_AUTO_TEST_CASE ( chunk_init_test )
{
	const char *needed[] = {"indexOrigin", "acquisitionNumber", "voxelSize", "rowVec", "columnVec"};
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataDebug, util::DefaultMsgPrint, warning );
	data::MemChunk<float> ch( 4, 3, 2, 1 );

	for( const char * str :  needed ) {
		BOOST_CHECK( ch.property( str ).isNeeded() );
	}


	BOOST_CHECK_EQUAL( ch.getVolume(), 1 * 2 * 3 * 4 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::rowDim ), 4 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::columnDim ), 3 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::sliceDim ), 2 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::timeDim ), 1 );
}


BOOST_AUTO_TEST_CASE ( chunk_index_test )
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	std::array<size_t,4> idx={1,1,1,0};
	const size_t at = 3 * 4 + 4 + 1;

	BOOST_CHECK_EQUAL( ch.getLinearIndex( idx ), at );
	BOOST_CHECK_EQUAL( idx, ch.getCoordsFromLinIndex( at ) );
}

BOOST_AUTO_TEST_CASE ( chunk_foreach_voxel_test )
{
	data::MemChunk<uint8_t> ch( 4, 3, 2, 1 );
	memset( &ch.asValueArray<uint8_t>()[0], 1, ch.getVolume() );

	class : public data::VoxelOp<uint8_t>
	{
	public:
		bool operator()( uint8_t &vox, const util::vector4<size_t>& /*pos*/ ) {
			return vox == 0;
		}
	} zero;

	class setIdx: public data::VoxelOp<uint8_t>
	{
		data::NDimensional<4> chunkGeometry;
	public:
		setIdx( data::NDimensional<4> geo ): chunkGeometry( geo ) {}
		bool operator()( uint8_t &vox, const util::vector4<size_t>& pos ) {
			vox = chunkGeometry.getLinearIndex( pos );
			return true;
		}
	};
	class checkIdx: public data::VoxelOp<uint8_t>
	{
		data::NDimensional<4> chunkGeometry;
	public:
		checkIdx( data::NDimensional<4> geo ): chunkGeometry( geo ) {}
		bool operator()( uint8_t &vox, const util::vector4<size_t>& pos ) {
			return vox == chunkGeometry.getLinearIndex( pos );
		}
	};

	BOOST_CHECK_EQUAL( ch.foreachVoxel( zero ), ch.getVolume() );
	memset( &ch.asValueArray<uint8_t>()[0], 0, ch.getVolume() );
	BOOST_CHECK_EQUAL( ch.foreachVoxel( zero ), 0 );

	checkIdx check( ch );
	setIdx set( ch );
	BOOST_CHECK_EQUAL( ch.foreachVoxel( check ), ch.getVolume() - 1 ); //the first index _is_ 0
	ch.foreachVoxel( set );
	BOOST_CHECK_EQUAL( ch.foreachVoxel( check ), 0 ); // now they all should be
}

BOOST_AUTO_TEST_CASE ( chunk_mem_init_test )
{
	const short data[3 * 3] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	data::MemChunk<short> ch( data, 3, 3 );
	BOOST_CHECK_EQUAL( ch.getVolume(), 3 * 3 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			BOOST_CHECK_EQUAL( ch.voxel<short>( i, j ), i + j * 3 );
}


BOOST_AUTO_TEST_CASE ( chunk_iterator_test )
{
	const short data[3 * 3] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	//hack to get generic Chunk instead of typed MemChunk
	//(MemChunk's storage is still ValueArray, so its shared/tracked anyway)
	data::Chunk ch(data::MemChunk<short>( data, 3, 3 ));

	BOOST_CHECK_EQUAL( ch.getVolume(), 3 * 3 );

	data::Chunk::const_iterator i = data::Chunk(ch).begin();

	for( ; i != ch.end(); ++i ) {
		const util::Value<short> datVal( data[std::distance( const_cast<const data::Chunk&>( ch ).begin(), i )] );
		BOOST_CHECK_EQUAL( *i, datVal );
	}

	const short *d=data;
	for(auto v:ch){
		BOOST_CHECK_EQUAL( v, util::Value<short>( *d++ ) );
	}
}

BOOST_AUTO_TEST_CASE ( chunk_voxel_value_test )
{
	const short data[3 * 3] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
	data::MemChunk<short> ch( data, 3, 3 );
	BOOST_CHECK_EQUAL( ch.getVolume(), 3 * 3 );

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ ) {
			BOOST_CHECK_EQUAL( ch.getVoxelValue( i, j )->as<int>(),  i + j * 3 );
			ch.setVoxelValue( util::Value<int>( i + j * 3 + 42 ), i, j );
		}

	for ( int i = 0; i < 3; i++ )
		for ( int j = 0; j < 3; j++ )
			BOOST_CHECK_EQUAL( ch.voxel<short>( i, j ), i + j * 3 + 42 );
}

BOOST_AUTO_TEST_CASE ( chunk_property_test )
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	//an basic Chunk must be invalid
	BOOST_CHECK( !ch.isValid() );
	BOOST_CHECK( !ch.hasProperty( "indexOrigin" ) );
	//with a position and an orientation its valid
	util::fvector3 pos( {1, 1, 1} );
	ch.setValueAs( "indexOrigin", pos );
	BOOST_CHECK( !ch.isValid() );
	ch.setValueAs<uint32_t>( "acquisitionNumber", 0 );
	BOOST_CHECK( !ch.isValid() );
	ch.setValueAs( "voxelSize", util::fvector3( {1, 1, 1} ) );
	BOOST_CHECK( !ch.isValid() );
	ch.setValueAs( "rowVec", pos );
	BOOST_CHECK( !ch.isValid() );
	ch.setValueAs( "columnVec", pos );
	BOOST_CHECK( ch.isValid() );
	//properties shall not be case sensitive
	BOOST_CHECK( ch.hasProperty( "indexorigin" ) );
	// and of course the property shall be what it was set to
	BOOST_CHECK_EQUAL( pos, ch.getValueAs<util::fvector3>( "indexOrigin" ) );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test1 )//Access Chunk elements via dimensional index
{
	data::MemChunk<float> ch( 4, 4, 4, 4 );

	for ( size_t i = 0; i < ch.getDimSize( data::rowDim ); i++ )
		ch.voxel<float>( i, i, i, i ) = i;

	for ( size_t i = 0; i < ch.getDimSize( data::rowDim ); i++ )
		BOOST_CHECK_EQUAL( ch.voxel<float>(i, i, i, i), i );

	data::Chunk ch2 = ch;

	for ( size_t i = 0; i < ch.getDimSize( data::rowDim ); i++ )
		BOOST_CHECK_EQUAL( ch2.voxel<float>(i, i, i, i), i );
}


BOOST_AUTO_TEST_CASE ( chunk_scale_test )//Access Chunk elements via dimensional index
{
	data::MemChunk<int16_t> ch( 10, 10, 10 );

	for ( size_t x = 0; x < ch.getDimSize( data::rowDim ); x++ ) {
		ch.voxel<int16_t>( x, x, 0 ) =  2500;
		ch.voxel<int16_t>( x, x, 1 ) = -50;
	}

	std::pair<util::ValueReference, util::ValueReference> minmax = ch.getMinMax();

	data::scaling_pair scale = ch.getScalingTo( data::ValueArray<uint8_t>::staticID(), minmax );
	const util::ValueBase &scale_s = *( scale.first );
	const util::ValueBase &scale_o = *( scale.second );

	BOOST_CHECK_EQUAL( scale_s.as<double>(), 1. / 10 );
	BOOST_CHECK_EQUAL( scale_o.as<double>(), 5 );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test2 )//Access Chunk elements via linear index (threat it as ValueArray)
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	std::ostringstream o;
	const size_t vol = 4 * 3 * 2 * 1;
	BOOST_REQUIRE_EQUAL( vol, ch.getVolume() );
	unsigned short sample[vol];

	for ( size_t i = 0; i < ch.getVolume(); i++ ) {
		ch.asValueArray<float>()[i] = i;
		sample[i] = i;
	}

	for ( size_t i = 0; i < ch.getVolume(); i++ )
		BOOST_CHECK( ch.getValueArray<float>()[i] == i );

	util::listToOStream(
		sample, sample + ch.getVolume(), o,
		"|",
		(std::to_string(ch.getVolume() ) + "#").c_str(), ""
	);
	BOOST_CHECK_EQUAL( o.str(), ch.getValueArray<float>().toString() );
}

BOOST_AUTO_TEST_CASE ( chunk_copy_test )//Copy chunks
{
	data::MemChunk<float> ch1( 4, 3, 2, 1 );
	ch1.setValueAs("test",1);

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValueArray<float>()[i] = i;

	data::scaling_pair no_scale( util::Value<int>( 1 ), util::Value<int>( 0 ) );

	data::Chunk ch2 = ch1;//This shall clone the underlying ValueArray-Object

	//make sure the properties are copied
	BOOST_CHECK_EQUAL( ch2.getValueAs<int>("test"),1);

	data::Chunk copyF = ch2.copyByID(); // this shall copy as the same as ch2 (float)
	data::Chunk copyI = ch2.copyByID( data::ValueArray<uint32_t>::staticID(), no_scale ); // this shall copy as unsigned int (we need to set scale because float=>int always scales up)

	//but it should of course be of the same type and contain the same data
	BOOST_CHECK( ch1.getValueArrayBase().is<float>() );
	BOOST_CHECK( ch1.getValueArrayBase().isSameType( ch2.getValueArrayBase() ) );
	BOOST_CHECK( copyF.getValueArrayBase().isSameType( ch2.getValueArrayBase() ) );
	BOOST_CHECK( copyI.getValueArrayBase().is<uint32_t>() );

	BOOST_CHECK_EQUAL( ch1.getVolume(), ch2.getVolume() );
	BOOST_CHECK_EQUAL( ch1.getVolume(), copyF.getVolume() );
	BOOST_CHECK_EQUAL( ch1.getVolume(), copyI.getVolume() );

	// all entries should be the same as for ch1
	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		BOOST_CHECK_EQUAL( ch2.getValueArray<float>()[i], i );
		BOOST_CHECK_EQUAL( copyF.getValueArray<float>()[i], i );
		BOOST_CHECK_EQUAL( copyI.getValueArray<uint32_t>()[i], i );
	}


	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		//cloning chunks is a cheap copy, thus any copied chunk shares data
		ch1.asValueArray<float>()[i] = 0;
		BOOST_CHECK_EQUAL( ch2.getValueArray<float>()[i], 0 );
		// but deep copies should not be changed
		BOOST_CHECK_EQUAL( copyF.getValueArray<float>()[i], i );
		BOOST_CHECK_EQUAL( copyI.getValueArray<uint32_t>()[i], i );
	}
}

BOOST_AUTO_TEST_CASE ( memchunk_copy_test )//Copy chunks
{
	static boost::numeric::converter <  short, double,
		   boost::numeric::conversion_traits<short, double>,
		   boost::numeric::def_overflow_handler,
		   boost::numeric::RoundEven<double>
		   > converter;
	data::MemChunk<float> ch1( 4, 3, 2, 1 );
	ch1.setValueAs( "indexOrigin", util::fvector3( {1, 2, 3} ) );

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValueArray<float>()[i] = i;

	data::MemChunk<short> ch2( ch1 );//This shall deep copy the chunk and convert the float data to short
	data::MemChunk<short> ch3( ch2 );//This shall deep copy the chunk without converting it
	//it should of course have the same size
	BOOST_CHECK_EQUAL( ch1.getVolume(), ch2.getVolume() );
	BOOST_CHECK_EQUAL( ch2.getVolume(), ch3.getVolume() );
	//it should have the same properties
	BOOST_REQUIRE( ch2.hasProperty( "indexOrigin" ) );
	BOOST_REQUIRE( ch3.hasProperty( "indexOrigin" ) );
	BOOST_CHECK_EQUAL( ch1.property( "indexOrigin" ), ch2.property( "indexOrigin" ) );
	BOOST_CHECK_EQUAL( ch2.property( "indexOrigin" ), ch3.property( "indexOrigin" ) );
	const float scale = float( std::numeric_limits< short >::max() ) / ( ch2.getVolume() - 1 );

	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		BOOST_CHECK_EQUAL( ch2.asValueArray<short>()[i], converter( i * scale ) );
		BOOST_CHECK_EQUAL( ch3.asValueArray<short>()[i], converter( i * scale ) );
	}

	data::MemChunk<short> ch4( 1, 1 );
	ch4 = ch3;
	BOOST_CHECK_EQUAL( ch3.getSizeAsVector(), ch4.getSizeAsVector() );

	//because MemChunk does deep copy changing ch3 should not change ch2
	for ( size_t i = 0; i < ch3.getVolume(); i++ ) {
		ch3.asValueArray<short>()[i] = 200;
		BOOST_CHECK_EQUAL( ch2.asValueArray<short>()[i], converter( i * scale ) );
		BOOST_CHECK_EQUAL( ch4.asValueArray<short>()[i], converter( i * scale ) );
	}
}

BOOST_AUTO_TEST_CASE ( chunk_splice_test )//Copy chunks
{
	data::MemChunk<float> ch1( 3, 3, 3 );
	ch1.setValueAs( "indexOrigin", util::fvector3( {1, 1, 1} ) );
	ch1.setValueAs( "rowVec", util::fvector3( {1, 0, 0} ) );
	ch1.setValueAs( "columnVec", util::fvector3( {0, 1, 0} ) );
	ch1.setValueAs( "voxelSize", util::fvector3( {1, 1, 1} ) );
	ch1.setValueAs( "voxelGap", util::fvector3( {1, 1, 1} ) );
	ch1.setValueAs<uint32_t>( "acquisitionNumber", 0 );

	const util::Value<int> buff[] = {0, 1, 2};
	std::copy( buff, buff + 3, std::back_inserter( ch1.touchProperty( "list_test" ) ) );


	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValueArray<float>()[i] = i;

	const std::list<data::Chunk> splices = ch1.autoSplice( 2 );
	unsigned short cnt = 0;
	BOOST_CHECK_EQUAL( splices.size(), 3 );
	for( const data::Chunk & ref :  splices ) {
		BOOST_CHECK_EQUAL( ref.property( "indexOrigin" ), util::fvector3( {1, 1, 1 + static_cast<float>(cnt * 2)} ) );
		BOOST_CHECK_EQUAL( ref.property( "list_test" ), cnt );
		BOOST_CHECK_EQUAL( ref.property( "acquisitionNumber" ), cnt*2 );// we ha a stride of 1, so acquisitionNumber should be 0 2 4 ..
		cnt++;
	}
}

BOOST_AUTO_TEST_CASE ( chunk_swap_test )
{
	class : public data::VoxelOp<int>
	{
		bool operator()( int &vox, const util::vector4<size_t> & ) {
			vox = rand();
			return true;
		}
	} randomize;
	class SwapCheck: public data::VoxelOp<int>
	{
		size_t swapidx, sizeRange;
	public:
		data::MemChunk<int> orig;
		SwapCheck( data::MemChunk<int> &_orig, size_t _swapidx, size_t _sizeRange ): swapidx( _swapidx ), sizeRange( _sizeRange ), orig( _orig ) {}
		bool operator()( int &vox, const util::vector4<size_t> &pos ) {
			util::vector4<size_t> opos = pos;
			opos[swapidx] = sizeRange - 1 - opos[swapidx];
			//          if(orig.voxel<int>(opos[0],opos[1],opos[2],opos[3])!=vox)
			//              std::cout << "Comparing " << pos << " against " << opos
			//                  << "(" << vox << "!=" << orig.voxel<int>(opos[0],opos[1],opos[2],opos[3])
			//                  << ")" << std::endl;
			return orig.voxel<int>( opos[0], opos[1], opos[2], opos[3] ) == vox;
		}
	};

	for ( int dim = data::rowDim; dim <= data::timeDim; dim++ ) { // for each dim
		for( size_t sizeRange = 10; sizeRange < 21; sizeRange++ ) { // check with chunks of the size 10³-21³
			//create chunk with random content
			data::MemChunk<int> ch1( sizeRange, sizeRange, sizeRange, sizeRange );
			ch1.foreachVoxel( randomize );

			//store a copy of the original data and the rest in the checker
			SwapCheck swap_check( ch1, data::rowDim, sizeRange );

			ch1.flipAlong( data::rowDim );//swap ch1
			BOOST_CHECK_EQUAL( ch1.foreachVoxel( swap_check ), 0 ); //run check for swapped ch1 and and original copy in swap_check

			ch1.flipAlong( data::rowDim );//swap it back
			BOOST_CHECK( ch1.compare( swap_check.orig ) == 0 ); //check for equality with the original copy in swap_check
		}
	}
}

BOOST_AUTO_TEST_CASE ( chunk_copySlice_Test )
{
	size_t rows = 13;
	size_t cols = 12;
	size_t slices = 24;
	size_t steps = 10;
	data::MemChunk<float> ch( cols, rows, slices, steps );

	for ( size_t ix = 0; ix < rows; ix++ ) {
		for ( size_t iy = 0; iy < cols; iy++ ) {
			for ( size_t is = 0; is < slices; is++ ) {
				for ( size_t it = 0; it < steps; it++ ) {
					ch.voxel<float>( iy, ix, is, it ) = ix + iy + is + it;
				}
			}
		}
	}

	data::MemChunk<float> chSlice( cols, rows );
	size_t slGet = 7;
	size_t stepGet = 1;
	ch.copySlice( slGet, stepGet, chSlice, 0, 0 );

	for( size_t i = 0; i < cols; i++ ) {
		for( size_t j = 0; j < rows; j++ ) {
			BOOST_CHECK_EQUAL( chSlice.voxel<float>( i, j, 0, 0 ), slGet + stepGet + i + j );
		}
	}

	//first slice
	data::MemChunk<float> chSlice5( cols, rows );
	slGet = 0;
	stepGet = 2;
	ch.copySlice( slGet, stepGet, chSlice5, 0, 0 );

	for( size_t i = 0; i < cols; i++ ) {
		for( size_t j = 0; j < rows; j++ ) {
			BOOST_CHECK_EQUAL( chSlice5.voxel<float>( i, j, 0, 0 ), slGet + stepGet + i + j );
		}
	}

	//lastslice
	data::MemChunk<float> chSlice6( cols, rows );
	slGet = 23;
	stepGet = 7;
	ch.copySlice( slGet, stepGet, chSlice6, 0, 0 );

	for( size_t i = 0; i < cols; i++ ) {
		for( size_t j = 0; j < rows; j++ ) {
			BOOST_CHECK_EQUAL( chSlice6.voxel<float>( i, j, 0, 0 ), slGet + stepGet + i + j );
		}
	}
}

BOOST_AUTO_TEST_CASE ( chunk_swapdim_test )
{
	//hack to get generic Chunk instead of typed MemChunk
	//(MemChunk's storage is still ValueArray, so its shared/tracked anyway)
	data::Chunk ch(data::MemChunk<uint32_t>( 50, 40, 30, 20 ));
	uint32_t cnt = 0;
	for( data::Chunk::reference ref :  ch )
	ref = util::Value<uint32_t>( cnt++ );

	data::MemChunk<uint32_t> swapped( ch );
	swapped.swapDim( data::columnDim, data::sliceDim );

	for( size_t t = 0; t < 20; t++ )
		for( size_t z = 0; z < 30; z++ )
			for( size_t y = 0; y < 40; y++ )
				for( size_t x = 0; x < 50; x++ ) {
					size_t idx = ch.getLinearIndex( {x, y, z, t} );
					BOOST_REQUIRE_EQUAL( ch.voxel<uint32_t>( x, y, z, t ), idx );
					BOOST_CHECK_EQUAL( swapped.voxel<uint32_t>( x, z, y, t ), idx );
				}

}

BOOST_AUTO_TEST_CASE ( typed_chunk_test )//Copy chunks
{
	data::MemChunk<float> ch1( 4, 3, 2, 1 );
	ch1.setValueAs("test",1);

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValueArray<float>()[i] = i+1;

	data::TypedChunk<float> ch2 = ch1;//This shall clone the underlying ValueArray-Object

	//make sure the properties are copied
	BOOST_CHECK_EQUAL( ch2.getValueAs<uint32_t>("test"),1);

	data::scaling_pair no_scale( util::Value<int>( 1 ), util::Value<int>( 0 ) );
	data::TypedChunk<uint32_t> copyI(ch2,no_scale); //This shall convert the underlying ValueArray-Object (without scaling it)

	//but it should of course be of the same type and contain the same data
	BOOST_CHECK( ch1.getValueArrayBase().is<float>() );
	BOOST_CHECK( ch1.getValueArrayBase().isSameType( ch2.getValueArrayBase() ) );
	BOOST_CHECK( copyI.getValueArrayBase().is<uint32_t>() );

	BOOST_CHECK_EQUAL( ch1.getVolume(), ch2.getVolume() );
	BOOST_CHECK_EQUAL( ch1.getVolume(), copyI.getVolume() );

	// all entries should be the same as for ch1
	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		BOOST_CHECK_EQUAL( ch2.getValueArray<float>()[i], i+1 );
		BOOST_CHECK_EQUAL( copyI.getValueArray<uint32_t>()[i], i+1 );
	}

	float v_sum=0;
	for ( auto v:ch2 ) {
		v_sum += v;
	}
	BOOST_CHECK_EQUAL(v_sum,ch2.getVolume()*(ch2.getVolume()+1)/2);
}


}
}
