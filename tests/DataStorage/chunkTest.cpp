/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ChunkTest
#include <boost/test/unit_test.hpp>
#include <DataStorage/chunk.hpp>
#include <boost/foreach.hpp>

namespace isis
{
namespace test
{

/* create an image */
BOOST_AUTO_TEST_CASE ( chunk_init_test )
{
	ENABLE_LOG( CoreLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( CoreDebug, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataLog, util::DefaultMsgPrint, warning );
	ENABLE_LOG( DataDebug, util::DefaultMsgPrint, warning );
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	BOOST_CHECK_EQUAL( ch.getVolume(), 1 * 2 * 3 * 4 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::rowDim ), 4 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::columnDim ), 3 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::sliceDim ), 2 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::timeDim ), 1 );
}


BOOST_AUTO_TEST_CASE ( chunk_index_test )
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	size_t idx[4], ridx[4];
	idx[0] = idx[1] = idx[2] = 1;
	idx[3] = 0;
	const size_t at = 3 * 4 + 4 + 1;
	ch.getCoordsFromLinIndex( at, ridx );


	BOOST_CHECK_EQUAL( ch.getLinearIndex( idx ), at );
	BOOST_CHECK( memcmp( idx, ridx, 4 ) == 0 );
}

BOOST_AUTO_TEST_CASE ( chunk_foreach_voxel_test )
{
	data::MemChunk<uint8_t> ch( 4, 3, 2, 1 );
	memset( &ch.asValuePtr<uint8_t>()[0], 1, ch.getVolume() );

	class : public data::VoxelOp<uint8_t>
	{
	public:
		bool operator()( uint8_t &vox, const util::vector4<size_t>& /*pos*/ ) {
			return vox == 0;
		}
	} zero;

	class setIdx: public data::VoxelOp<uint8_t>
	{
		data::_internal::NDimensional<4> chunkGeometry;
	public:
		setIdx( data::_internal::NDimensional<4> geo ): chunkGeometry( geo ) {}
		bool operator()( uint8_t &vox, const util::vector4<size_t>& pos ) {
			vox = chunkGeometry.getLinearIndex( &pos[0] );
			return true;
		}
	};
	class checkIdx: public data::VoxelOp<uint8_t>
	{
		data::_internal::NDimensional<4> chunkGeometry;
	public:
		checkIdx( data::_internal::NDimensional<4> geo ): chunkGeometry( geo ) {}
		bool operator()( uint8_t &vox, const util::vector4<size_t>& pos ) {
			return vox == chunkGeometry.getLinearIndex( &pos[0] );
		}
	};

	BOOST_CHECK_EQUAL( ch.foreachVoxel( zero ), ch.getVolume() );
	memset( &ch.asValuePtr<uint8_t>()[0], 0, ch.getVolume() );
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

BOOST_AUTO_TEST_CASE ( chunk_property_test )
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	//an basic Chunk must be invalid
	BOOST_CHECK( !ch.isValid() );
	BOOST_CHECK( !ch.hasProperty( "indexOrigin" ) );
	//with a position and an orientation its valid
	util::fvector4 pos( 1, 1, 1 );
	ch.setPropertyAs( "indexOrigin", pos );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs( "rowVec", pos );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs( "columnVec", pos );
	BOOST_CHECK( ch.isValid() );
	//properties shall not be case sensitive
	BOOST_CHECK( ch.hasProperty( "indexorigin" ) );
	// and of course the property shall be what it was set to
	BOOST_CHECK_EQUAL( pos, ch.getPropertyAs<util::fvector4>( "indexOrigin" ) );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test1 )//Access Chunk elements via dimensional index
{
	data::MemChunk<float> ch( 4, 4, 4, 4 );

	for ( size_t i = 0; i < ch.getDimSize( data::rowDim ); i++ )
		ch.voxel<float>( i, i, i, i ) = i;

	for ( size_t i = 0; i < ch.getDimSize( data::rowDim ); i++ )
		BOOST_CHECK_EQUAL( ch.voxel<float>( i, i, i, i ), i );

	data::Chunk ch2 = ch;

	for ( size_t i = 0; i < ch.getDimSize( data::rowDim ); i++ )
		BOOST_CHECK_EQUAL( ch2.voxel<float>( i, i, i, i ), i );
}


BOOST_AUTO_TEST_CASE ( chunk_scale_test )//Access Chunk elements via dimensional index
{
	data::MemChunk<int16_t> ch( 10, 10, 10 );

	for ( size_t x = 0; x < ch.getDimSize( data::rowDim ); x++ ) {
		ch.voxel<int16_t>( x, x, 0 ) =  2500;
		ch.voxel<int16_t>( x, x, 1 ) = -50;
	}

	std::pair<util::ValueReference, util::ValueReference> minmax = ch.getMinMax();

	data::scaling_pair scale = ch.getScalingTo( data::ValuePtr<uint8_t>::staticID, minmax );
	const util::_internal::ValueBase &scale_s = *( scale.first );
	const util::_internal::ValueBase &scale_o = *( scale.second );

	BOOST_CHECK_EQUAL( scale_s.as<double>(), 1. / 10 );
	BOOST_CHECK_EQUAL( scale_o.as<double>(), 5 );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test2 )//Access Chunk elements via linear index (threat it as ValuePtr)
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	std::ostringstream o;
	const size_t vol = 4 * 3 * 2 * 1;
	BOOST_REQUIRE_EQUAL( vol, ch.getVolume() );
	unsigned short sample[vol];

	for ( size_t i = 0; i < ch.getVolume(); i++ ) {
		ch.asValuePtr<float>()[i] = i;
		sample[i] = i;
	}

	for ( size_t i = 0; i < ch.getVolume(); i++ )
		BOOST_CHECK( ch.getValuePtr<float>()[i] == i );

	util::listToOStream(
		sample, sample + ch.getVolume(), o,
		"|",
		util::Value<int32_t>( ch.getVolume() ).toString( false ) + "#", ""
	);
	BOOST_CHECK_EQUAL( o.str(), ch.getValuePtr<float>().toString() );
}

BOOST_AUTO_TEST_CASE ( chunk_copy_test )//Copy chunks
{
	data::MemChunk<float> ch1( 4, 3, 2, 1 );

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValuePtr<float>()[i] = i;

	data::Chunk ch2 = ch1;//This shall clone the underlying ValuePtr-Object
	//but it should of course of the same type and contain the same data
	BOOST_CHECK( ch1.getValuePtrBase().isSameType( ch2.getValuePtrBase() ) );
	BOOST_CHECK( ch1.getValuePtrBase().is<float>() );
	BOOST_CHECK_EQUAL( ch1.getVolume(), ch2.getVolume() );

	for ( size_t i = 0; i < ch2.getVolume(); i++ )
		BOOST_CHECK_EQUAL( ch2.getValuePtr<float>()[i], i );

	//cloning chunks is a cheap copy, thus any copied chunk shares data
	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		ch1.asValuePtr<float>()[i] = 0;
		BOOST_CHECK_EQUAL( ch2.getValuePtr<float>()[i], 0 );
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
	ch1.setPropertyAs( "indexOrigin", util::fvector4( 1, 2, 3, 4 ) );

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValuePtr<float>()[i] = i;

	data::MemChunk<short> ch2( ch1 );//This shall deep copy the chunk and convert the float data to short
	data::MemChunk<short> ch3( ch2 );//This shall deep copy the chunk without converting it
	//it should of course have the same size
	BOOST_CHECK_EQUAL( ch1.getVolume(), ch2.getVolume() );
	BOOST_CHECK_EQUAL( ch2.getVolume(), ch3.getVolume() );
	//it should have the same properties
	BOOST_REQUIRE( ch2.hasProperty( "indexOrigin" ) );
	BOOST_REQUIRE( ch3.hasProperty( "indexOrigin" ) );
	BOOST_CHECK_EQUAL( ch1.propertyValue( "indexOrigin" ), ch2.propertyValue( "indexOrigin" ) );
	BOOST_CHECK_EQUAL( ch2.propertyValue( "indexOrigin" ), ch3.propertyValue( "indexOrigin" ) );
	const float scale = float( std::numeric_limits< short >::max() ) / ( ch2.getVolume() - 1 );

	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		BOOST_CHECK_EQUAL( ch2.asValuePtr<short>()[i], converter( i * scale ) );
		BOOST_CHECK_EQUAL( ch3.asValuePtr<short>()[i], converter( i * scale ) );
	}

	data::MemChunk<short> ch4( 1, 1 );
	ch4 = ch3;
	BOOST_CHECK_EQUAL( ch3.getSizeAsVector(), ch4.getSizeAsVector() );

	//because MemChunk does deep copy changing ch3 should not change ch2
	for ( size_t i = 0; i < ch3.getVolume(); i++ ) {
		ch3.asValuePtr<short>()[i] = 200;
		BOOST_CHECK_EQUAL( ch2.asValuePtr<short>()[i], converter( i * scale ) );
		BOOST_CHECK_EQUAL( ch4.asValuePtr<short>()[i], converter( i * scale ) );
	}
}

BOOST_AUTO_TEST_CASE ( chunk_splice_test )//Copy chunks
{
	data::MemChunk<float> ch1( 3, 3, 3 );
	ch1.setPropertyAs( "indexOrigin", util::fvector4( 1, 1, 1 ) );
	ch1.setPropertyAs( "rowVec", util::fvector4( 1, 0, 0 ) );
	ch1.setPropertyAs( "columnVec", util::fvector4( 0, 1, 0 ) );
	ch1.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
	ch1.setPropertyAs( "voxelGap", util::fvector4( 1, 1, 1 ) );
	ch1.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asValuePtr<float>()[i] = i;

	const std::list<data::Chunk> splices = ch1.autoSplice( );
	unsigned short cnt = 1;
	BOOST_CHECK_EQUAL( splices.size(), 3 );
	BOOST_FOREACH( const data::Chunk & ref, splices ) {
		BOOST_CHECK_EQUAL( ref.getPropertyAs<util::fvector4>( "indexOrigin" ), util::fvector4( 1, 1, cnt ) );
		cnt += 2;
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

			ch1.swapAlong( data::rowDim );//swap ch1
			BOOST_CHECK_EQUAL( ch1.foreachVoxel( swap_check ), 0 ); //run check for swapped ch1 and and original copy in swap_check

			ch1.swapAlong( data::rowDim );//swap it back
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

}
}
