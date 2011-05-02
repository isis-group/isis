#include "DataStorage/io_application.hpp"
#include "DataStorage/io_factory.hpp"

#include <map>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/io.hpp>

using namespace isis;

template<typename TYPE>
data::Image voxelFlipZ( const data::Image &src, unsigned int dim )
{
	data::Image tmpImage = data::MemImage<TYPE> ( src );

	for ( size_t t = 0; t < src.getSizeAsVector()[3]; t++ ) {
		for ( size_t z = 0; z < src.getSizeAsVector()[2]; z++ ) {
			for ( size_t y = 0; y < src.getSizeAsVector()[1]; y++ ) {
				for ( size_t x = 0; x < src.getSizeAsVector()[0]; x++ ) {
					tmpImage.voxel<TYPE>( x, y, z, t ) = src.voxel<TYPE>( dim == 0 ? ( src.getSizeAsVector()[0] - x ) - 1 : x,
														 dim == 1 ? ( src.getSizeAsVector()[1] - y ) - 1 : y,
														 dim == 2 ? ( src.getSizeAsVector()[2] - z ) - 1 : z,
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
		( "row", 0 ) ( "phase", 1 ) ( "slice", 2 );
	data::IOApplication app( "isisflip", true, true );
	util::Selection along( "x,y,z,row,column,slice" );
	util::Selection flip( "image,space,both" );
	along.set( "x" );
	flip.set( "both" );
	app.parameters["image_center"] = bool();
	app.parameters["image_center"].needed() = false;
	app.parameters["image_center"].setDescription("If activated the center of the image will be translated to the of the scanner space and after flipping back to its initial position" );
	app.parameters["image_center"] = false;
	app.parameters["along"] = along;
	app.parameters["along"].needed() = true;
	app.parameters["along"].setDescription( "Flip along the specified axis" );
	app.parameters["flip"] = flip;
	app.parameters["flip"].needed() = true;
	app.parameters["flip"].setDescription( "What has to be flipped" );
	app.init( argc, argv );
	std::list<data::Image> finImageList;
	unsigned int dim = alongMap[app.parameters["along"].toString()];
	//go through every image
	BOOST_FOREACH( data::Image & refImage, app.images ) {
		//map from pyhisical into image space
		util::fvector4 sliceVec = refImage.getPropertyAs<util::fvector4>( "sliceVec" );
		util::fvector4 columnVec = refImage.getPropertyAs<util::fvector4>( "columnVec" );
		util::fvector4 rowVec = refImage.getPropertyAs<util::fvector4>( "rowVec" );
		util::fvector4 f1( rowVec[0], columnVec[0], sliceVec[0], 0  );
		util::fvector4 f2( rowVec[1], columnVec[1], sliceVec[1], 0  );
		util::fvector4 f3( rowVec[2], columnVec[2], sliceVec[2], 0  );
		boost::numeric::ublas::matrix<float> T = boost::numeric::ublas::identity_matrix<float>( 3, 3 );
		T( dim, dim ) *= -1;
		data::Image newImage = refImage;

		if ( app.parameters["flip"].toString() == "image" || app.parameters["flip"].toString() == "both" ) {
			switch ( refImage.getMajorTypeID() ) {
			case data::ValuePtr<uint8_t>::staticID:
				newImage = voxelFlipZ<uint8_t>( refImage, dim );
				break;
			case data::ValuePtr<int8_t>::staticID:
				newImage = voxelFlipZ<int8_t>( refImage, dim );
				break;
			case data::ValuePtr<uint16_t>::staticID:
				newImage = voxelFlipZ<uint16_t>( refImage, dim );
				break;
			case data::ValuePtr<int16_t>::staticID:
				newImage = voxelFlipZ<int16_t>( refImage, dim );
				break;
			case data::ValuePtr<uint32_t>::staticID:
				newImage = voxelFlipZ<uint32_t>( refImage, dim );
				break;
			case data::ValuePtr<int32_t>::staticID:
				newImage = voxelFlipZ<int32_t>( refImage, dim );
				break;
			case data::ValuePtr<uint64_t>::staticID:
				newImage = voxelFlipZ<uint64_t>( refImage, dim );
				break;
			case data::ValuePtr<int64_t>::staticID:
				newImage = voxelFlipZ<int64_t>( refImage, dim );
				break;
			case data::ValuePtr<float>::staticID:
				newImage = voxelFlipZ<float>( refImage, dim );
				break;
			case data::ValuePtr<double>::staticID:
				newImage = voxelFlipZ<double>( refImage, dim );
				break;
			default:
				break;
			}
		}
		if ( app.parameters["flip"].toString() == "both" || app.parameters["flip"].toString() == "space" ) {
			newImage.transformCoords( T, app.parameters["image_center"] );
		}
		finImageList.push_back(  newImage );
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
