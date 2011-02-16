/*
* imageTest.cpp
*
*  Created on: Oct 1, 2009
*      Author: proeger
*/

#define BOOST_TEST_MODULE ChunkTest
#define BOOST_TEST_DYN_LINK
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
	BOOST_CHECK_EQUAL( ch.getDimSize( data::readDim ), 4 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::phaseDim ), 3 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::sliceDim ), 2 );
	BOOST_CHECK_EQUAL( ch.getDimSize( data::timeDim ), 1 );
}

BOOST_AUTO_TEST_CASE ( chunk_foreach_voxel_test )
{
	data::MemChunk<uint8_t> ch( 4, 3, 2, 1 );
	memset(&ch.asTypePtr<uint8_t>()[0],1,ch.getVolume());

	class :public data::Chunk::VoxelOp<uint8_t>{
	public:
		bool operator()(uint8_t& vox, const util::FixedVector< size_t, 4 >& pos){
			return vox==0;
		}
	}zero;

	class setIdx:public data::Chunk::VoxelOp<uint8_t>{
		data::_internal::NDimensional<4> chunkGeometry;
	public:
		setIdx(data::_internal::NDimensional<4> geo):chunkGeometry(geo){}
		bool operator()(uint8_t& vox, const util::FixedVector< size_t, 4 >& pos){
			vox=chunkGeometry.getLinearIndex(&pos[0]);
			return true;
		}
	};
	class checkIdx:public data::Chunk::VoxelOp<uint8_t>{
		data::_internal::NDimensional<4> chunkGeometry;
	public:
		checkIdx(data::_internal::NDimensional<4> geo):chunkGeometry(geo){}
		bool operator()(uint8_t& vox, const util::FixedVector< size_t, 4 >& pos){
			return vox==chunkGeometry.getLinearIndex(&pos[0]);
		}
	};

	BOOST_CHECK_EQUAL(ch.foreachVoxel(zero),ch.getVolume());
	memset(&ch.asTypePtr<uint8_t>()[0],0,ch.getVolume());
	BOOST_CHECK_EQUAL(ch.foreachVoxel(zero),0);

	checkIdx check(ch);
	setIdx set(ch);
	BOOST_CHECK_EQUAL(ch.foreachVoxel(check),ch.getVolume()-1); //the first index _is_ 0
	ch.foreachVoxel(set);
	BOOST_CHECK_EQUAL(ch.foreachVoxel(check),0); // now they all should be
}

BOOST_AUTO_TEST_CASE ( chunk_mem_init_test )
{
	const short data[3*3] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
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
	//with an position and an orientation its valid
	util::fvector4 pos( 1, 1, 1 );
	ch.setPropertyAs( "indexOrigin", pos );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1, 0 ) );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs( "readVec", pos );
	BOOST_CHECK( !ch.isValid() );
	ch.setPropertyAs( "phaseVec", pos );
	BOOST_CHECK( ch.isValid() );
	//properties shall not be case sensitive
	BOOST_CHECK( ch.hasProperty( "indexorigin" ) );
	// and of course the property shall be what it was set to
	BOOST_CHECK_EQUAL( pos, ch.getPropertyAs<util::fvector4>( "indexOrigin" ) );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test1 )//Access Chunk elements via dimensional index
{
	data::MemChunk<float> ch( 4, 4, 4, 4 );

	for ( size_t i = 0; i < ch.getDimSize( data::readDim ); i++ )
		ch.voxel<float>( i, i, i, i ) = i;

	for ( size_t i = 0; i < ch.getDimSize( data::readDim ); i++ )
		BOOST_CHECK_EQUAL( ch.voxel<float>( i, i, i, i ), i );

	data::Chunk ch2 = ch;

	for ( size_t i = 0; i < ch.getDimSize( data::readDim ); i++ )
		BOOST_CHECK_EQUAL( ch2.voxel<float>( i, i, i, i ), i );
}


BOOST_AUTO_TEST_CASE ( chunk_scale_test )//Access Chunk elements via dimensional index
{
	data::MemChunk<int16_t> ch( 10, 10, 10 );

	for ( size_t x = 0; x < ch.getDimSize( data::readDim ); x++ ) {
		ch.voxel<int16_t>( x, x, 0 ) =  2500;
		ch.voxel<int16_t>( x, x, 1 ) = -50;
	}

	std::pair<util::TypeReference, util::TypeReference> minmax = ch.getMinMax();

	data::scaling_pair scale = ch.getScalingTo( data::TypePtr<uint8_t>::staticID, *minmax.first, *minmax.second );
	const util::_internal::TypeBase &scale_s = *( scale.first );
	const util::_internal::TypeBase &scale_o = *( scale.second );

	BOOST_CHECK_EQUAL( scale_s.as<double>(), 1. / 10 );
	BOOST_CHECK_EQUAL( scale_o.as<double>(), 5 );
}

BOOST_AUTO_TEST_CASE ( chunk_data_test2 )//Access Chunk elements via linear index (threat it as TypePtr)
{
	data::MemChunk<float> ch( 4, 3, 2, 1 );
	std::ostringstream o;
	const size_t vol = 4 * 3 * 2 * 1;
	BOOST_REQUIRE_EQUAL( vol, ch.getVolume() );
	unsigned short sample[vol];

	for ( size_t i = 0; i < ch.getVolume(); i++ ) {
		ch.asTypePtr<float>()[i] = i;
		sample[i] = i;
	}

	for ( size_t i = 0; i < ch.getVolume(); i++ )
		BOOST_CHECK( ch.getTypePtr<float>()[i] == i );

	util::listToOStream(
		sample, sample + ch.getVolume(), o,
		"|",
		util::Value<int32_t>( ch.getVolume() ).toString( false ) + "#", ""
	);
	BOOST_CHECK_EQUAL( o.str(), ch.getTypePtr<float>().toString() );
}

BOOST_AUTO_TEST_CASE ( chunk_copy_test )//Copy chunks
{
	data::MemChunk<float> ch1( 4, 3, 2, 1 );

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asTypePtr<float>()[i] = i;

	data::Chunk ch2 = ch1;//This shall clone the underlying TypePtr-Object
	//but it should of course of the same type and contain the same data
	BOOST_CHECK( ch1.getTypePtrBase().isSameType( ch2.getTypePtrBase() ) );
	BOOST_CHECK( ch1.getTypePtrBase().is<float>() );
	BOOST_CHECK_EQUAL( ch1.getVolume(), ch2.getVolume() );

	for ( size_t i = 0; i < ch2.getVolume(); i++ )
		BOOST_CHECK_EQUAL( ch2.getTypePtr<float>()[i], i );

	//cloning chunks is a cheap copy, thus any copied chunk shares data
	for ( size_t i = 0; i < ch2.getVolume(); i++ ) {
		ch1.asTypePtr<float>()[i] = 0;
		BOOST_CHECK_EQUAL( ch2.getTypePtr<float>()[i], 0 );
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
		ch1.asTypePtr<float>()[i] = i;

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
		BOOST_CHECK_EQUAL( ch2.asTypePtr<short>()[i], converter( i * scale ) );
		BOOST_CHECK_EQUAL( ch3.asTypePtr<short>()[i], converter( i * scale ) );
	}

	data::MemChunk<short> ch4( 1, 1 );
	ch4 = ch3;
	BOOST_CHECK_EQUAL( ch3.getSizeAsVector(), ch4.getSizeAsVector() );

	//because MemChunk does deep copy changing ch3 should not change ch2
	for ( size_t i = 0; i < ch3.getVolume(); i++ ) {
		ch3.asTypePtr<short>()[i] = 200;
		BOOST_CHECK_EQUAL( ch2.asTypePtr<short>()[i], converter( i * scale ) );
		BOOST_CHECK_EQUAL( ch4.asTypePtr<short>()[i], converter( i * scale ) );
	}
}

BOOST_AUTO_TEST_CASE ( chunk_splice_test )//Copy chunks
{
	data::MemChunk<float> ch1( 3, 3, 3 );
	ch1.setPropertyAs( "indexOrigin", util::fvector4( 1, 1, 1 ) );
	ch1.setPropertyAs( "readVec", util::fvector4( 1, 0, 0 ) );
	ch1.setPropertyAs( "phaseVec", util::fvector4( 0, 1, 0 ) );
	ch1.setPropertyAs( "voxelSize", util::fvector4( 1, 1, 1 ) );
	ch1.setPropertyAs( "voxelGap", util::fvector4( 1, 1, 1 ) );
	ch1.setPropertyAs<uint32_t>( "acquisitionNumber", 0 );

	for ( size_t i = 0; i < ch1.getVolume(); i++ )
		ch1.asTypePtr<float>()[i] = i;

	const std::list<data::Chunk> splices = ch1.autoSplice( );
	unsigned short cnt = 1;
	BOOST_CHECK_EQUAL( splices.size(), 3 );
	BOOST_FOREACH( const data::Chunk &ref, splices ) {
		BOOST_CHECK_EQUAL( ref.getPropertyAs<util::fvector4>( "indexOrigin" ), util::fvector4( 1, 1, cnt ) );
		cnt += 2;
	}
}

BOOST_AUTO_TEST_CASE ( chunk_swap_test )
{
	//TODO
	//  data::MemChunk<float> ch1( 3, 3, 3, 1 );
	//  ch1.setPropertyAs( "indexOrigin", util::fvector4( 1, 1, 1, 1 ) );
	//  ch1.setPropertyAs( "readVec", util::fvector4( 1, 0, 0 ) );
	//  ch1.setPropertyAs( "phaseVec", util::fvector4( 0, 1, 0 ) );
	//  ch1.setPropertyAs( "sliceVec", util::fvector4( 0, 0, 1 ) );
	//  ch1.setPropertyAs( "dummyProp", std::string( "dummy" ) );
	//
	//  for ( size_t i = 0; i < ch1.getVolume(); i++ )
	//      ch1.asTypePtr<float>()[i] = i;
	//
	//  data::MemChunk<float> ch2( 3, 3, 3, 1 );
	//  data::MemChunk<float> ch3( 3, 3, 3, 1 );
	//
	//  //transform will not be changed...
	//  for ( size_t dim = 0; dim < 3; dim++ ) {
	//      ch1.swapAlong( ch2, dim, false );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "indexOrigin" ), ch2.getPropertyAs<util::fvector4>( "indexOrigin" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "readVec" ), ch2.getPropertyAs<util::fvector4>( "readVec" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "phaseVec" ), ch2.getPropertyAs<util::fvector4>( "phaseVec" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "sliceVec" ), ch2.getPropertyAs<util::fvector4>( "sliceVec" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<std::string>( "dummyProp" ), ch2.getPropertyAs<std::string>( "dummyProp" ) );
	//      ch2.swapAlong( ch3, dim, false );
	//
	//      for ( size_t i = 0; i < ch1.getVolume(); i++ ) {
	//          BOOST_CHECK_EQUAL( ch1.asTypePtr<float>()[i], ch3.asTypePtr<float>()[i] );
	//      }
	//  }
	//
	//  //transform will be changed
	//  for ( size_t dim = 0; dim < 3; dim++ ) {
	//      ch1.swapAlong( ch2, dim, true );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "indexOrigin" )[dim], -ch2.getPropertyAs<util::fvector4>( "indexOrigin" )[dim] );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "readVec" )[dim], -ch2.getPropertyAs<util::fvector4>( "readVec" )[dim] );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "phaseVec" )[dim], -ch2.getPropertyAs<util::fvector4>( "phaseVec" )[dim] );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "sliceVec" )[dim], -ch2.getPropertyAs<util::fvector4>( "sliceVec" )[dim] );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<std::string>( "dummyProp" ), ch2.getPropertyAs<std::string>( "dummyProp" ) );
	//      ch2.swapAlong( ch3, dim, true );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "indexOrigin" ), ch3.getPropertyAs<util::fvector4>( "indexOrigin" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "readVec" ), ch3.getPropertyAs<util::fvector4>( "readVec" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "phaseVec" ), ch3.getPropertyAs<util::fvector4>( "phaseVec" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<util::fvector4>( "sliceVec" ), ch3.getPropertyAs<util::fvector4>( "sliceVec" ) );
	//      BOOST_CHECK_EQUAL( ch1.getPropertyAs<std::string>( "dummyProp" ), ch3.getPropertyAs<std::string>( "dummyProp" ) );
	//
	//      for ( size_t i = 0; i < ch1.getVolume(); i++ ) {
	//          BOOST_CHECK_EQUAL( ch1.asTypePtr<float>()[i], ch3.asTypePtr<float>()[i] );
	//      }
	//  }
}

BOOST_AUTO_TEST_CASE ( chunk_copyLine_Test )
{
	size_t rows = 23;
	size_t cols = 17;
	size_t slices = 14;
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

	data::MemChunk<float> chRow( 1, cols );
	data::MemChunk<float> chRowT( cols, 1 );
	size_t rowGet = 9;
	size_t slGet = 7;
	size_t stepGet = 1;
	ch.copyLine( rowGet, slGet, stepGet, chRow, 0, 0, 0 );
	ch.copyLine( rowGet, slGet, stepGet, chRowT, 0, 0, 0 );

	for( size_t c = 0; c < cols; c++ ) {
		BOOST_CHECK_EQUAL( chRow.voxel<float>( 0, c, 0, 0 ), rowGet + slGet + stepGet + c );
		BOOST_CHECK_EQUAL( chRowT.voxel<float>( c, 0, 0, 0 ), rowGet + slGet + stepGet + c );
	}

	rows = 13;
	cols = 17;
	slices = 4;
	steps = 1;
	data::MemChunk<float> ch2( cols, rows, slices, steps );

	for ( size_t ix = 0; ix < rows; ix++ ) {
		for ( size_t iy = 0; iy < cols; iy++ ) {
			for ( size_t is = 0; is < slices; is++ ) {
				for ( size_t it = 0; it < steps; it++ ) {
					ch2.voxel<float>( iy, ix, is, it ) = ix + iy + is + it;
				}
			}
		}
	}

	data::MemChunk<float> chRow2( 1, cols );
	rowGet = 9;
	slGet = 2;
	stepGet = 0;
	ch2.copyLine( rowGet, slGet, stepGet, chRow2, 0, 0, 0 );

	for( size_t c = 0; c < cols; c++ ) {
		BOOST_CHECK_EQUAL( chRow2.voxel<float>( 0, c, 0, 0 ), rowGet + slGet + stepGet + c );
	}

	rows = 1;
	cols = 12;
	slices = 4;
	steps = 1;
	data::MemChunk<float> ch3( cols, rows, slices, steps );

	for ( size_t ix = 0; ix < rows; ix++ ) {
		for ( size_t iy = 0; iy < cols; iy++ ) {
			for ( size_t is = 0; is < slices; is++ ) {
				for ( size_t it = 0; it < steps; it++ ) {
					ch3.voxel<float>( iy, ix, is, it ) = ix + iy + is + it;
				}
			}
		}
	}

	data::MemChunk<float> chRow3( 1, cols );
	rowGet = 0;
	slGet = 2;
	stepGet = 0;
	ch3.copyLine( rowGet, slGet, stepGet, chRow3, 0, 0, 0 );

	for( size_t c = 0; c < cols; c++ ) {
		BOOST_CHECK_EQUAL( chRow3.voxel<float>( 0, c, 0, 0 ), rowGet + slGet + stepGet + c );
	}

	rows = 18;
	cols = 1;
	slices = 7;
	steps = 11;
	data::MemChunk<float> ch4( cols, rows, slices, steps );

	for ( size_t ix = 0; ix < rows; ix++ ) {
		for ( size_t iy = 0; iy < cols; iy++ ) {
			for ( size_t is = 0; is < slices; is++ ) {
				for ( size_t it = 0; it < steps; it++ ) {
					ch4.voxel<float>( iy, ix, is, it ) = ix + iy + is + it;
				}
			}
		}
	}

	data::MemChunk<float> chRow4( 1, cols );
	rowGet = 0;
	slGet = 2;
	stepGet = 0;
	ch4.copyLine( rowGet, slGet, stepGet, chRow4, 0, 0, 0 );

	for( size_t c = 0; c < cols; c++ ) {
		BOOST_CHECK_EQUAL( chRow4.voxel<float>( 0, c, 0, 0 ), rowGet + slGet + stepGet + c );
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

	//out of size
	//TODO:: leerer Chunk vs. chunk mit vorhandenen Werten
	data::MemChunk<float> chSlice3( cols, rows );
	slGet = 25;
	stepGet = 1;
	ch.copySlice( slGet, stepGet, chSlice3, 0, 0 );

	for( size_t i = 0; i < cols; i++ ) {
		for( size_t j = 0; j < rows; j++ ) {
			//  BOOST_CHECK_EQUAL( chSlice3.voxel<float>( i, j, 0, 0 ), 0 );
		}
	}

	data::MemChunk<float> chSlice4( cols, rows );
	slGet = 22;
	stepGet = 10;
	ch.copySlice( slGet, stepGet, chSlice4, 0, 0 );

	for( size_t i = 0; i < cols; i++ ) {
		for( size_t j = 0; j < rows; j++ ) {
			//  BOOST_CHECK_EQUAL( chSlice4.voxel<float>( i, j, 0, 0 ), 0 );
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
