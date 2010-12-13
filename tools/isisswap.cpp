#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"

#include <map>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>

using namespace isis;

template<typename TYPE>
data::Image voxelSwapZ( const boost::shared_ptr<data::Image> src, unsigned int dim )
{
	data::Image tmpImage = data::MemImage<TYPE> ( *src );

	for ( size_t t = 0; t < src->getSizeAsVector()[3]; t++ ) {
		for ( size_t z = 0; z < src->getSizeAsVector()[2]; z++ ) {
			for ( size_t y = 0; y < src->getSizeAsVector()[1]; y++ ) {
				for ( size_t x = 0; x < src->getSizeAsVector()[0]; x++ ) {
					tmpImage.voxel<TYPE>( x, y, z, t ) = src->voxel<TYPE>( dim == 0 ? ( src->getSizeAsVector()[0] - x ) - 1 : x,
														 dim == 1 ? ( src->getSizeAsVector()[1] - y ) - 1 : y,
														 dim == 2 ? ( src->getSizeAsVector()[2] - z ) - 1 : z,
														 t );
				}
			}
		}
	}

	return tmpImage;
}

int main( int argc, char **argv )
{
	ENABLE_LOG( data::Runtime, util::DefaultMsgPrint, error );
	const size_t getBiggestVecElem( const util::fvector4 & vec );
	std::map<std::string, unsigned int> alongMap = boost::assign::map_list_of
			( "x", 0 ) ( "y", 1 ) ( "z", 2 ) ( "sagittal", 3 ) ( "coronal", 4 ) ( "axial", 5 );
	data::IOApplication app( "isisswap", true, true );
	util::Selection along( "x,y,z,sagittal,coronal,axial" );
	util::Selection swap( "image,space,both" );
	along.set( "x" );
	swap.set( "both" );
	app.parameters["along"] = along;
	app.parameters["along"].needed() = true;
	app.parameters["along"].setDescription( "Swap along the specified axis" );
	app.parameters["swap"] = swap;
	app.parameters["swap"].needed() = true;
	app.parameters["swap"].setDescription( "What has to be swapped" );
	app.init( argc, argv );
	data::ChunkList finChunkList;
	data::ImageList finImageList;
	unsigned int dim = alongMap[app.parameters["along"].toString()];
	//go through every image
	BOOST_FOREACH( data::ImageList::const_reference refImage, app.images ) {
		//map from pyhisical into image space
		util::fvector4 sliceVec = refImage->getPropertyAs<util::fvector4>( "sliceVec" );
		util::fvector4 phaseVec = refImage->getPropertyAs<util::fvector4>( "phaseVec" );
		util::fvector4 readVec = refImage->getPropertyAs<util::fvector4>( "readVec" );
		util::fvector4 f1( readVec[0], phaseVec[0], sliceVec[0], 0  );
		util::fvector4 f2( readVec[1], phaseVec[1], sliceVec[1], 0  );
		util::fvector4 f3( readVec[2], phaseVec[2], sliceVec[2], 0  );

		if( dim == 3 ) {
			dim = getBiggestVecElem( f1 );
		}

		if( dim == 4 ) {
			dim = getBiggestVecElem( f2 );
		}

		if( dim == 5 ) {
			dim = getBiggestVecElem( f3 );
		}

		boost::numeric::ublas::matrix<float> T( 3, 3 );
		T( 0, 0 ) = 1;
		T( 0, 1 ) = 0;
		T( 0, 2 ) = 0;
		T( 1, 0 ) = 0;
		T( 1, 1 ) = 1;
		T( 1, 2 ) = 0;
		T( 2, 0 ) = 0;
		T( 2, 1 ) = 0;
		T( 2, 2 ) = 1;
		T( dim, dim ) *= -1;
		data::Image newImage = *refImage;

		if ( app.parameters["swap"].toString() == "image" || app.parameters["swap"].toString() == "both" ) {
			switch ( refImage->typeID() ) {
			case data::TypePtr<uint8_t>::staticID:
				newImage = voxelSwapZ<uint8_t>( refImage, dim );
				break;
			case data::TypePtr<int8_t>::staticID:
				newImage = voxelSwapZ<int8_t>( refImage, dim );
				break;
			case data::TypePtr<uint16_t>::staticID:
				newImage = voxelSwapZ<uint16_t>( refImage, dim );
				break;
			case data::TypePtr<int16_t>::staticID:
				newImage = voxelSwapZ<int16_t>( refImage, dim );
				break;
			case data::TypePtr<uint32_t>::staticID:
				newImage = voxelSwapZ<uint32_t>( refImage, dim );
				break;
			case data::TypePtr<int32_t>::staticID:
				newImage = voxelSwapZ<int32_t>( refImage, dim );
				break;
			case data::TypePtr<uint64_t>::staticID:
				newImage = voxelSwapZ<uint64_t>( refImage, dim );
				break;
			case data::TypePtr<int64_t>::staticID:
				newImage = voxelSwapZ<int64_t>( refImage, dim );
				break;
			case data::TypePtr<float>::staticID:
				newImage = voxelSwapZ<float>( refImage, dim );
				break;
			case data::TypePtr<double>::staticID:
				newImage = voxelSwapZ<double>( refImage, dim );
				break;
			default:
				break;
			}
		}

		if ( app.parameters["swap"].toString() == "both" || app.parameters["swap"].toString() == "space" ) {
			newImage.transformCoords( T );
			std::vector<boost::shared_ptr< data::Chunk> > chList = newImage.getChunkList();
			BOOST_FOREACH( std::vector<boost::shared_ptr< data::Chunk> >::reference chRef, chList ) {
				chRef->transformCoords( T );
			}
		}

		finImageList.push_back( boost::shared_ptr<data::Image> ( new data::Image( newImage ) ) );
	}
	app.autowrite( finImageList );
	return 0;
};


const size_t getBiggestVecElem( const util::fvector4 &vec )
{
	size_t biggestVecElem = 0;
	float tmpValue = 0;

	for ( size_t vecElem = 0; vecElem < 4; vecElem++ ) {
		if ( fabs( vec[vecElem] ) > fabs( tmpValue ) ) {
			biggestVecElem = vecElem;
			tmpValue = vec[vecElem];
		}
	}

	return biggestVecElem;
}
