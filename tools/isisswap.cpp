#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"

#include <map>
#include <boost/assign.hpp>

using namespace isis;

template<typename TYPE>
data::Image voxelSwapZ( const boost::shared_ptr<data::Image> src )
{
	data::MemImage<TYPE> dest( *src );
	data::Image tmpImage = dest;
	for ( size_t t = 0; t < src->sizeToVector()[3]; t++ ) {
		for ( size_t z = 0; z < src->sizeToVector()[2]; z++ ) {
			for ( size_t y = 0; y < src->sizeToVector()[1]; y++ ) {
				for ( size_t x = 0; x < src->sizeToVector()[0]; x++ ) {
					tmpImage.voxel<TYPE>(x, y, z, t) = src->voxel<TYPE>(x, y, (src->sizeToVector()[2] - z)-1, t);
					std::cout << tmpImage.voxel<TYPE>(x, y, z, t);
				}
			}
		}
	}
	return tmpImage;
}

int main( int argc, char **argv )
{
	ENABLE_LOG( data::Runtime, util::DefaultMsgPrint, error);

	const size_t getBiggestVecElem( const util::fvector4& vec);

	std::map<std::string, unsigned int> alongMap = boost::assign::map_list_of
			("x", 0) ("y", 1) ("z", 2) ("sagittal", 3) ("coronal", 4) ("axial", 5);

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
		//if we have a 4d image not organized at least as dim[3] chunks we have to splice
		//down to the timeDim. Now we have at least 3d images we can transform
		if( refImage->sizeToVector()[3] > refImage->getChunkList().size() ) {
			refImage->spliceDownTo(data::timeDim);
		}
		//map from pyhisical into image space
		if(dim == 3) {
			dim = getBiggestVecElem( refImage->getProperty<util::fvector4>("readVec") );
		}
		if(dim == 4) {
			dim = getBiggestVecElem( refImage->getProperty<util::fvector4>("phaseVec") );
		}
		if(dim == 5) {
			dim = getBiggestVecElem( refImage->getProperty<util::fvector4>("sliceVec") );
		}
		data::Image newImage;
		if ( refImage->getChunkAt(0).relevantDims() < 3 && dim == 2 ) {
			switch (refImage->typeID()) {
			case data::TypePtr<u_int8_t>::staticID:
				newImage = voxelSwapZ<u_int8_t>( refImage );
				break;
			case data::TypePtr<int8_t>::staticID:
				newImage = voxelSwapZ<int8_t>( refImage );
				break;
			case data::TypePtr<u_int16_t>::staticID:
				newImage = voxelSwapZ<u_int16_t>( refImage );
				break;
			case data::TypePtr<int16_t>::staticID:
				newImage = voxelSwapZ<int16_t>( refImage );
				break;
			case data::TypePtr<u_int32_t>::staticID:
				newImage = voxelSwapZ<u_int32_t>( refImage );
				break;
			case data::TypePtr<int32_t>::staticID:
				newImage = voxelSwapZ<int32_t>( refImage );
				break;
			case data::TypePtr<u_int64_t>::staticID:
				newImage = voxelSwapZ<u_int64_t>( refImage );
				break;
			case data::TypePtr<int64_t>::staticID:
				newImage = voxelSwapZ<int64_t>( refImage );
				break;
			case data::TypePtr<float>::staticID:
				newImage = voxelSwapZ<float>( refImage );
				break;
			case data::TypePtr<double>::staticID:
				newImage = voxelSwapZ<double>( refImage );
				break;
			default:
				break;
			}
			finImageList.push_back(boost::shared_ptr<data::Image> ( new data::Image( newImage ) ) );
		} else {
			std::vector<boost::shared_ptr<data::Chunk> > chList = refImage->getChunkList();
			BOOST_FOREACH( std::vector<boost::shared_ptr< data::Chunk> >::reference chRef, chList ) {
				chRef->join( static_cast<util::PropMap>( *refImage ), false );
				data::Chunk tmpChunk = chRef->cloneToMem( chRef->sizeToVector()[0], chRef->sizeToVector()[1], chRef->sizeToVector()[2], chRef->sizeToVector()[3] );

				if ( app.parameters["swap"].toString() == "both" ) {
					if ( !chRef->swapAlong( tmpChunk, dim, true ) ) {
						LOG( data::Runtime, error) << "Swapping failed.";
					}
				} else if ( app.parameters["swap"].toString() == "image" ) {

					if ( !chRef->swapAlong( tmpChunk, dim, false ) ) {
						LOG( data::Runtime, error) << "Swapping failed.";
					}
				} else if ( app.parameters["swap"].toString() == "space" ) {
					tmpChunk = *chRef;
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
					tmpChunk.transformCoords( T );
				}

				finChunkList.push_back( boost::shared_ptr<data::Chunk>( new data::Chunk( tmpChunk ) ) );
			}
			finImageList = data::ImageList(finChunkList);
		}
	}
	app.autowrite( finImageList );
	return 0;
};




const size_t getBiggestVecElem( const util::fvector4& vec)
{
	size_t biggestVecElem = 0;
	float tmpValue = 0;
	for ( size_t vecElem = 0; vecElem < 4; vecElem++) {
		if ( fabs(vec[vecElem]) > fabs(tmpValue) ) {
 			biggestVecElem = vecElem;
			tmpValue = vec[vecElem];
		}
	}
	return biggestVecElem;
}
