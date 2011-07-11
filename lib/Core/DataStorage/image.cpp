// kate: show-indent on; indent-mode tab; indent-width 4; tab-width 4; replace-tabs off; auto-insert-doxygen on

//
// C++ Implementation: image
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifdef _MSC_VER
#pragma warning(disable:4996 4244)
#endif

#include "image.hpp"
#include "../CoreUtils/vector.hpp"
#include <boost/foreach.hpp>
#include "../CoreUtils/property.hpp"
#include <boost/token_iterator.hpp>

#define _USE_MATH_DEFINES 1
#include <math.h>
#include <cmath>

namespace isis
{
namespace data
{

Image::Image ( ) : set( "sequenceNumber,rowVec,columnVec,sliceVec,coilChannelMask,DICOM/EchoNumbers" ), clean( false )
{
	addNeededFromString( neededProperties );
	set.addSecondarySort( "acquisitionNumber" );
	set.addSecondarySort( "acquisitionTime" );
}

Image::Image ( const Chunk &chunk, dimensions min_dim ) :
	_internal::NDimensional<4>(), util::PropertyMap(), minIndexingDim( min_dim ),
	set( "sequenceNumber,rowVec,columnVec,coilChannelMask,DICOM/EchoNumbers" ),
	clean( false )
{
	addNeededFromString( neededProperties );
	set.addSecondarySort( "acquisitionNumber" );
	set.addSecondarySort( "acquisitionTime" );

	if ( ! ( insertChunk( chunk ) && reIndex() && isClean() ) ) {
		LOG( Runtime, error ) << "Failed to create image from single chunk.";
	} else if( !isValid() ) {
		LOG_IF( !getMissing().empty(), Debug, warning )
				<< "The created image is missing some properties: " << getMissing() << ". It will be invalid.";
	}
}

Image::Image( const isis::data::Image &ref ): _internal::NDimensional<4>(), util::PropertyMap(),
	set( "" )/*SortedChunkList has no default constructor - lets just make an empty (and invalid) set*/
{
	( *this ) = ref; // set will be replaced here anyway
}

Image &Image::operator=( const isis::data::Image &ref )
{
	//deep copy bases
	static_cast<util::PropertyMap &>( *this ) = static_cast<const util::PropertyMap &>( ref );
	static_cast<_internal::NDimensional< 4 >&>( *this ) = static_cast<const _internal::NDimensional< 4 >&>( ref );
	//deep copy members
	chunkVolume = ref.chunkVolume;
	clean = ref.clean;
	set = ref.set;
	minIndexingDim = ref.minIndexingDim;
	//replace all chunks (in set) by cheap copies of them
	struct : public _internal::SortedChunkList::chunkPtrOperator {
		boost::shared_ptr< Chunk > operator()( const boost::shared_ptr< Chunk >& ptr ) {
			return boost::shared_ptr< Chunk > ( new Chunk( *ptr ) );
		}
	} replace;
	set.transform( replace );
	lookup = set.getLookup();
	return *this;
}


bool Image::checkMakeClean()
{
	if ( ! clean ) {
		LOG( Debug, info )  << "Image is not clean. Running reIndex ...";

		if( !reIndex() ) {
			LOG( Runtime, error ) << "Reindexing failed -- undefined behavior ahead ...";
		}
	}

	return clean;
}
bool Image::isClean()const
{
	return clean;
}

void Image::deduplicateProperties()
{
	LOG_IF( lookup.empty(), Debug, error ) << "The lookup table is empty. Won't do anything.";
	const boost::shared_ptr<Chunk> &first = lookup[0];
	//@todo might fail if the image contains a prop that differs to that in the Chunks (which is equal in the chunks)
	util::PropertyMap common;
	util::PropertyMap::KeyList uniques;
	first->toCommonUnique( common, uniques, true );

	for ( size_t i = 1; i < lookup.size(); i++ ) {
		lookup[i]->toCommonUnique( common, uniques, false );
	}

	LOG( Debug, info ) << uniques.size() << " Chunk-unique properties found in the Image";
	LOG_IF( uniques.size(), Debug, verbose_info ) << util::listToString( uniques.begin(), uniques.end(), ", " );
	join( common );
	LOG_IF( ! common.isEmpty(), Debug, verbose_info ) << "common properties saved into the image " << common;

	//remove common props from the chunks
	for ( size_t i = 0; i != lookup.size(); i++ )
		lookup[i]->remove( common, false ); //this _won't keep needed properties - so from here on the chunks of the image are invalid

	LOG_IF( ! common.isEmpty(), Debug, verbose_info ) << "common properties removed from " << lookup.size() << " chunks: " << common;

}

bool Image::insertChunk ( const Chunk &chunk )
{
	if ( chunk.getVolume() == 0 ) {
		LOG( Runtime, error )
				<< "Cannot insert empty Chunk (Size is " << chunk.getSizeAsString() << ").";
		return false;
	}

	if ( ! chunk.isValid() ) {
		LOG( Runtime, error )
				<< "Cannot insert invalid chunk. Missing properties: " << chunk.getMissing();
		return false;
	}

	if( clean ) {
		LOG( Debug, info ) << "Resetting image structure because of new insertion.";
		LOG( Runtime, warning ) << "Inserting into already indexed images is inefficient. You should not do that.";

		// re-gather all properties of the chunks from the image
		BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, lookup ) {
			ref->join( *this );
		}
	}


	LOG_IF( chunk.getPropertyAs<util::fvector4>( "indexOrigin" )[3] != 0, Debug, warning )
			<< " inserting chunk with nonzero at the 4th position - you shouldn't use the fourth dim for the time (use acquisitionTime)";

	if( set.insert( chunk ) ) { // if the insertion was successful the image has to be reindexed anyway
		clean = false;
		lookup.clear();
		return true;
	} else {
		// if the insersion failed but the image was clean - de-duplicate properties again
		// the image is still clean - no need reindex
		if( clean )
			deduplicateProperties();

		return false;
	}
}

void Image::setIndexingDim( dimensions d )
{
	minIndexingDim = d;

	if( clean ) {
		LOG( Debug, warning ) << "Image was allready indexed. reIndexing ...";
		reIndex();
	}
}

util::fvector4 Image::getPhysicalCoordsFromIndex( const isis::util::ivector4 &voxelCoords ) const
{
	return  util::fvector4( voxelCoords[0] * m_RowVec[0] + voxelCoords[1] * m_ColumnVec[0] + voxelCoords[2] * m_SliceVec[0],
							voxelCoords[0] * m_RowVec[1] + voxelCoords[1] * m_ColumnVec[1] + voxelCoords[2] * m_SliceVec[1],
							voxelCoords[0] * m_RowVec[2] + voxelCoords[1] * m_ColumnVec[2] + voxelCoords[2] * m_SliceVec[2],
							voxelCoords[3] )
			+ m_Offset ;
}




util::ivector4 Image::getIndexFromPhysicalCoords( const isis::util::fvector4 &physicalCoords ) const
{
	util::fvector4 vec1 = physicalCoords - m_Offset;
	util::fvector4 ret = util::fvector4( vec1[0] * m_RowVecInv[0] + vec1[1] * m_ColumnVecInv[0] + vec1[2] * m_SliceVecInv[0],
										 vec1[0] * m_RowVecInv[1] + vec1[1] * m_ColumnVecInv[1] + vec1[2] * m_SliceVecInv[1],
										 vec1[0] * m_RowVecInv[2] + vec1[1] * m_ColumnVecInv[2] + vec1[2] * m_SliceVecInv[2],
										 vec1[3] );
	return  util::Value<util::fvector4>( ret ).as<util::ivector4>();
}


bool Image::updateOrientationMatrices()
{
	util::fvector4 rowVec = getPropertyAs<util::fvector4>( "rowVec" );
	util::fvector4 columnVec = getPropertyAs<util::fvector4>( "columnVec" );
	util::fvector4 sliceVec = getPropertyAs<util::fvector4>( "sliceVec" );
	m_Offset = getPropertyAs<util::fvector4>( "indexOrigin" );
	util::fvector4 spacing = getPropertyAs<util::fvector4>( "voxelSize" ) + ( hasProperty( "voxelGap" ) ? getPropertyAs<util::fvector4>( "voxelGap" ) : util::fvector4( 0, 0, 0 ) );
	m_RowVec = util::fvector4( rowVec[0] * spacing[0], rowVec[1] * spacing[0], rowVec[2] * spacing[0] );
	m_ColumnVec = util::fvector4( columnVec[0] * spacing[1], columnVec[1] * spacing[1], columnVec[2] * spacing[1] );
	m_SliceVec = util::fvector4( sliceVec[0] * spacing[2], sliceVec[1] * spacing[2], sliceVec[2] * spacing[2] );
	LOG( Debug, verbose_info ) << "Created orientation matrix: ";
	LOG( Debug, verbose_info ) << "[ " << m_RowVec[0] << " " << m_ColumnVec[0] << " " << m_SliceVec[0] << " ] + " << m_Offset[0];
	LOG( Debug, verbose_info ) << "[ " << m_RowVec[1] << " " << m_ColumnVec[1] << " " << m_SliceVec[1] << " ] + " << m_Offset[1];
	LOG( Debug, verbose_info ) << "[ " << m_RowVec[2] << " " << m_ColumnVec[2] << " " << m_SliceVec[2] << " ] + " << m_Offset[2];

	//since we do not want to calculate the inverse matrix with every getVoxelCoords call again we do it here once
	for ( size_t i = 0; i < 3; i++ ) {
		spacing[i] = spacing[i] ? 1.0 / spacing[i] : 0;
	}

	//for inversion of the orientation we use boost::ublas
	using namespace boost::numeric::ublas;
	matrix<float> orientation = matrix<float>( 3, 3 );
	matrix<float> inverse = matrix<float>( 3, 3 );

	for( size_t i = 0; i < 3; i++ ) {
		orientation( i, 0 ) = m_RowVec[i];
		orientation( i, 1 ) = m_ColumnVec[i];
		orientation( i, 2 ) = m_SliceVec[i];
	}

	if( !_internal::inverseMatrix<float>( orientation, inverse ) ) {
		LOG( Runtime, error ) << "Could not create the inverse of the orientation matrix!";
		return false;
	};

	for( size_t i = 0; i < 3; i++ ) {
		m_RowVecInv[i] = inverse( i, 0 );
		m_ColumnVecInv[i] = inverse( i, 1 );
		m_SliceVecInv[i] = inverse( i, 2 );
	}

	LOG( Debug, verbose_info ) << "Created transposed orientation matrix: ";
	LOG( Debug, verbose_info ) << "[ " << m_RowVecInv[0] << " " << m_ColumnVecInv[0] << " " << m_SliceVecInv[0] << " ] + " << m_Offset[0];
	LOG( Debug, verbose_info ) << "[ " << m_RowVecInv[1] << " " << m_ColumnVecInv[1] << " " << m_SliceVecInv[1] << " ] + " << m_Offset[1];
	LOG( Debug, verbose_info ) << "[ " << m_RowVecInv[2] << " " << m_ColumnVecInv[2] << " " << m_SliceVecInv[2] << " ] + " << m_Offset[2];
	return true;
}


dimensions Image::mapScannerAxesToImageDimension( scannerAxis scannerAxes )
{
	updateOrientationMatrices();
	boost::numeric::ublas::matrix<float> latchedOrientation = boost::numeric::ublas::zero_matrix<float>( 3, 3 );
	boost::numeric::ublas::vector<float>mapping( 3 );
	latchedOrientation( m_RowVec.getBiggestVecElemAbs(), 0 ) = 1;
	latchedOrientation( m_ColumnVec.getBiggestVecElemAbs(), 1 ) = 1;
	latchedOrientation( m_SliceVec.getBiggestVecElemAbs(), 2 ) = 1;

	for( size_t i = 0; i < 3; i++ ) {
		mapping( i ) = i;
	}

	return static_cast<dimensions>( boost::numeric::ublas::prod( latchedOrientation, mapping )( scannerAxes ) );



}


bool Image::reIndex()
{
	if ( set.isEmpty() ) {
		LOG( Debug, warning ) << "Reindexing an empty image is useless.";
		return false;
	}

	if( !set.isRectangular() ) {
		LOG( Runtime, error ) << "The image is incomplete. Aborting reindex. (horizontal size is " << set.getHorizontalSize() << ")";
		return false;
	}

	//redo lookup table
	lookup = set.getLookup();
	util::FixedVector<size_t, dims> structure_size; //storage for the size of the chunk structure
	structure_size.fill( 1 );
	//get primary attributes from geometrically first chunk - will be usefull
	const Chunk &first = chunkAt( 0 );
	//start indexing at eigther the chunk-size or the givem minIndexingDim (whichever is bigger)
	const unsigned short chunk_dims = std::max<unsigned short>( first.getRelevantDims(), minIndexingDim );
	chunkVolume = first.getVolume();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Determine structure of the image by searching for dimensional breaks in the chunklist
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//get indexOrigin from the geometrically first chunk
	propertyValue( "indexOrigin" ) = first.propertyValue( "indexOrigin" );
	//if there are many chunks, they must leave at least on dimension to the image to "sort" them in
	const size_t timesteps = set.getHorizontalSize();
	const unsigned short sortDims = dims - ( timesteps > 1 ? 1 : 0 ); // dont use the uppermost dim, if the timesteps are already there

	if ( chunk_dims >= Image::dims ) {
		if ( lookup.size() > 1 ) {
			LOG( Runtime, error )
					<< "Cannot handle multiple Chunks, if they have more than "
					<< Image::dims - 1 << " dimensions";
			return false;
		}

		//if there is only one chunk, its ok - the image will consist only of this one,
		//and commonGet will allways return <0,set.begin()->getLinearIndex()>
		//thus in this case voxel() equals set.begin()->voxel()
	} else {// OK there is at least one dimension to sort in the chunks
		LOG( Debug, info ) << "Computing strides for dimensions " << util::MSubject( chunk_dims + 1 ) << " to " << util::MSubject( sortDims );

		// check the chunks for at least one dimensional break - use that for the size of that dimension
		for ( unsigned short i = chunk_dims; i < sortDims; i++ ) { //if there are dimensions left figure out their size
			structure_size[i] = getChunkStride( structure_size.product() ) / structure_size.product();
			assert( structure_size[i] != 0 );
		}
	}

	if ( sortDims < dims ) { //if there is a timedim (not all dims was used for geometric sort)
		assert( structure_size[sortDims] == 1 );
		structure_size[sortDims] = timesteps; // fill the dim above the top geometric dim with the timesteps
	}

	assert( structure_size.product() == lookup.size() );
	//Clean up the properties
	deduplicateProperties();

	// add the chunk-size to the image-size
	for ( unsigned short i = 0; i < chunk_dims; i++ )
		structure_size[i] = first.getDimSize( i );

	init( structure_size ); // set size of the image
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//reconstruct some redundant information, if its missing
	//////////////////////////////////////////////////////////////////////////////////////////////////
	const util::PropertyMap::KeyType vectors[] = {"rowVec", "columnVec", "sliceVec"};
	int oneCnt = 0;
	BOOST_FOREACH( const util::PropertyMap::KeyType & ref, vectors ) {
		if ( hasProperty( ref ) ) {
			util::PropertyValue &prop = propertyValue( ref );
			LOG_IF( !prop->is<util::fvector4>(), Debug, error ) << "Using " << prop->getTypeName() << " as " << util::Value<util::fvector4>::staticName();
			util::fvector4 &vec = prop->castTo<util::fvector4>();

			if( vec.sqlen() == 0 ) {
				util::fvector4  v_one;
				v_one[oneCnt] = 1;
				LOG( Runtime, error )
						<< "The existing " << ref << " " << vec << ( hasProperty( "source" ) ? " from " + getPropertyAs<std::string>( "source" ) : "" ) << " has the length zero. Falling back to " << v_one << ".";
				vec = v_one;
			}

			vec.norm();
		}

		oneCnt++;
	}

	//if we have at least two slides (and have slides (with different positions) at all)
	if ( chunk_dims == 2 && structure_size[2] > 1 && first.hasProperty( "indexOrigin" ) ) {
		const util::fvector4 thisV = first.getPropertyAs<util::fvector4>( "indexOrigin" );
		const Chunk &last = chunkAt( structure_size[2] - 1 );

		if ( last.hasProperty( "indexOrigin" ) ) {
			const util::fvector4 lastV = last.getPropertyAs<util::fvector4>( "indexOrigin" );
			//check the slice vector
			util::fvector4 distVecNorm = lastV - thisV;
			LOG_IF( distVecNorm.len() == 0, Runtime, error )
					<< "The distance between the the first and the last chunk is zero. Thats bad, because I'm going to normalize it.";
			distVecNorm.norm();

			if ( hasProperty( "sliceVec" ) ) {
				const util::fvector4 sliceVec = getPropertyAs<util::fvector4>( "sliceVec" );
				LOG_IF( ! distVecNorm.fuzzyEqual( sliceVec ), Runtime, info )
						<< "The existing sliceVec " << sliceVec
						<< " differs from the distance vector between chunk 0 and " << structure_size[2] - 1
						<< " " << distVecNorm;
			} else {
				LOG( Debug, info )
						<< "used the distance between chunk 0 and " << structure_size[2] - 1
						<< " to synthesize the missing sliceVec as " << distVecNorm;
				propertyValue( "sliceVec" ) = distVecNorm;
			}
		}

		const util::fvector4 &voxeSize = getPropertyAs<util::fvector4>( "voxelSize" );

		const Chunk &next = chunkAt( 1 );

		if ( next.hasProperty( "indexOrigin" ) ) {
			const util::fvector4 nextV = next.getPropertyAs<util::fvector4>( "indexOrigin" );
			const float sliceDist = ( nextV - thisV ).len() - voxeSize[2];

			if ( sliceDist > 0 ) {
				const float inf = std::numeric_limits<float>::infinity();

				if ( ! hasProperty( "voxelGap" ) ) { // @todo check this
					setPropertyAs( "voxelGap", util::fvector4( 0, 0, inf, 0 ) );
				}

				util::fvector4 &voxelGap = propertyValue( "voxelGap" )->castTo<util::fvector4>(); //if there is no voxelGap yet, we create it

				if ( voxelGap[2] != inf ) {
					if ( ! util::fuzzyEqual( voxelGap[2], sliceDist ) ) {
						LOG_IF( ! util::fuzzyEqual( voxelGap[2], sliceDist ), Runtime, warning )
								<< "The existing slice distance (voxelGap[2]) " << util::MSubject( voxelGap[2] )
								<< " differs from the distance between chunk 0 and 1, which is " << sliceDist;
					}
				} else {
					voxelGap[2] = sliceDist;
					LOG( Debug, info )
							<< "used the distance between chunk 0 and 1 to synthesize the missing slice distance (voxelGap[2]) as "
							<< sliceDist;
				}
			}
		}
	}

	//if we have row- and column- vector
	if ( hasProperty( "rowVec" ) && hasProperty( "columnVec" ) ) {
		util::fvector4 &row = propertyValue( "rowVec" )->castTo<util::fvector4>();
		util::fvector4 &column = propertyValue( "columnVec" )->castTo<util::fvector4>();
		LOG_IF( row.dot( column ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
		const util::fvector4 crossVec = util::fvector4( //we could use their cross-product as sliceVector
											row[1] * column[2] - row[2] * column[1],
											row[2] * column[0] - row[0] * column[2],
											row[0] * column[1] - row[1] * column[0]
										);

		if ( hasProperty( "sliceVec" ) ) {
			util::fvector4 &sliceVec = propertyValue( "sliceVec" )->castTo<util::fvector4>(); //get the slice vector
			LOG_IF( ! crossVec.fuzzyEqual( sliceVec ), Runtime, warning )
					<< "The existing sliceVec " << sliceVec
					<< " differs from the cross product of the row- and column vector " << crossVec;
		} else {
			// We dont know anything about the slice-direction
			// we just guess its along the positive cross-product between row- and column direction
			// so at least warn the user if we do that long shot
			LOG( Runtime, info )
					<< "used the cross product between rowVec and columnVec as sliceVec:"
					<< crossVec << ". That might be wrong!";
			setPropertyAs( "sliceVec", crossVec );
		}
	}

	if ( hasProperty( "fov" ) ) {
		util::fvector4 &propFoV = propertyValue( "fov" )->castTo<util::fvector4>();

		const util::fvector4 &calcFoV = getFoV();

		bool ok = true;

		for ( size_t i = 0; i < dims; i++ ) {
			if ( propFoV[i] != -std::numeric_limits<float>::infinity() ) {
				ok &= util::fuzzyEqual( propFoV[i], calcFoV[i] );
			} else
				propFoV[i] = calcFoV[i];
		}

		LOG_IF( ! ok, Runtime, info )
				<< "The calculated field of view differs from the stored " << propFoV << "/" << calcFoV;
	}

	LOG_IF( ! isValid(), Runtime, warning ) << "The image is not valid after reindexing. Missing properties: " << getMissing();
	updateOrientationMatrices();
	return clean = isValid();
}
bool Image::isEmpty()const
{
	return set.isEmpty();
}

const boost::shared_ptr< Chunk >& Image::chunkPtrAt( size_t pos )const
{
	LOG_IF( lookup.empty(), Debug, error ) << "The lookup table is empty. Run reIndex first.";
	LOG_IF( pos >= lookup.size(), Debug, error ) << "Index is out of the range of the lookup table (" << pos << ">=" << lookup.size() << ").";
	const boost::shared_ptr<Chunk> &ptr = lookup[pos];
	LOG_IF( !ptr, Debug, error ) << "There is no chunk at " << pos << ". This usually happens in incomplete images.";
	return ptr;
}

Chunk Image::getChunkAt( size_t pos, bool copy_metadata )const
{
	Chunk ret( *chunkPtrAt( pos ) );

	if( copy_metadata )ret.join( *this ); // copy all metadata from the image in here

	return ret;
}
Chunk &Image::chunkAt( size_t pos )
{
	return *chunkPtrAt( pos );
}

Chunk Image::getChunk ( size_t first, size_t second, size_t third, size_t fourth, bool copy_metadata )
{
	checkMakeClean();
	return const_cast<const Image &>( *this ).getChunk( first, second, third, fourth, copy_metadata ); // use the const version
}

const Chunk Image::getChunk ( size_t first, size_t second, size_t third, size_t fourth, bool copy_metadata ) const
{
	const size_t index = commonGet( first, second, third, fourth ).first;
	return getChunkAt( index, copy_metadata );
}

std::vector< Chunk > Image::copyChunksToVector( bool copy_metadata )const
{
	std::vector<isis::data::Chunk> ret;
	ret.reserve( lookup.size() );
	std::vector<boost::shared_ptr<Chunk> >::const_iterator at = lookup.begin();
	const std::vector<boost::shared_ptr<Chunk> >::const_iterator end = lookup.end();

	while ( at != end ) {
		ret.push_back( **( at++ ) );

		if( copy_metadata )
			ret.back().join( *this );
	}

	return ret;
}

size_t Image::getChunkStride ( size_t base_stride )
{
	LOG_IF( set.isEmpty(), Runtime, error ) << "Trying to get chunk stride in an empty image";
	LOG_IF( lookup.empty(), Debug, error ) << "Lookup table for chunks is empty. Do reIndex() first!";

	if ( lookup.size() >= 4 * base_stride ) {
		/* there can't be any stride with less than 3*base_stride chunks (which would actually be an invalid image)
		 * _____
		 * |c c| has no stride/dimensional break
		 * _____
		 * |c c|
		 * |c  | has a dimensional break, but is invalid
		 * _____
		 * |c c|
		 * |c c| is the first reasonable case
		 */
		// get the distance between first and second chunk for comparision
		const util::fvector4 firstV = chunkAt( 0 ).getPropertyAs<util::fvector4>( "indexOrigin" );
		const util::fvector4 secondV = chunkAt( base_stride ).getPropertyAs<util::fvector4>( "indexOrigin" );
		const util::fvector4 dist1 = secondV - firstV;

		if( dist1.sqlen() == 0 ) { //if there is no geometric structure anymore - so asume its flat from here on
			LOG( Debug, info ) << "Distance between 0 and " << util::MSubject( base_stride )
							   << " is zero. Assuming there are no dimensional breaks anymore. Returning " << util::MSubject( base_stride );
			return base_stride;
		} else for ( size_t i = base_stride; i < lookup.size() - base_stride; i += base_stride ) {  // compare every follwing distance to that
				const util::fvector4 thisV = chunkAt( i ).getPropertyAs<util::fvector4>( "indexOrigin" );
				const util::fvector4 nextV = chunkAt( i + base_stride ).getPropertyAs<util::fvector4>( "indexOrigin" );
				const util::fvector4 distFirst = nextV - firstV;
				const util::fvector4 distThis = nextV - thisV;
				LOG( Debug, verbose_info )
						<< "Distance between chunk " << util::MSubject( i ) << " and " << util::MSubject( i + base_stride )
						<< " is " << distThis.len() << ". Distance between 0 and " << util::MSubject( i + base_stride ) << " is " << distFirst.len();

				if ( distFirst.sqlen() <= distThis.sqlen() ) { // the next chunk is nearer to the begin than to this => dimensional break => leave
					LOG( Debug, info )
							<< "Distance between chunk " << util::MSubject( i + base_stride )
							<< " and 0 is not bigger than the distance between " << util::MSubject( i + base_stride )
							<< " and " << util::MSubject( i ) << ", assuming dimensional break at " << i + base_stride;
					return i + base_stride;
				}
			}
	} else  if ( lookup.size() % base_stride ) {
		LOG( Runtime, error )
				<< "The amount of chunks (" << lookup.size()
				<< ") is not divisible by the block size of the dimension below (" << base_stride
				<< "). Maybe the image is incomplete.";
		LOG( Runtime, warning )
				<< "Ignoring "  <<  lookup.size() % base_stride << " chunks.";
		return lookup.size() - ( lookup.size() % base_stride );
	}

	//we didn't find any break, so we assume its a linear image |c c ... c|
	LOG( Debug, info )
			<< "No dimensional break found, assuming it to be at the end (" << lookup.size() << "/" << set.getHorizontalSize() << ")";
	return lookup.size() / set.getHorizontalSize();
}

std::list<util::PropertyValue> Image::getChunksProperties( const util::PropertyMap::KeyType &key, bool unique )const
{
	std::list<util::PropertyValue > ret;

	if( clean ) {
		BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
			const util::PropertyValue &prop = ref->propertyValue( key );

			if ( unique && prop.isEmpty() ) //if unique is requested and the property is empty
				continue; //skip it
			else if ( unique && !( ret.empty() ||  prop == ret.back() ) )
				//if unique is requested and the property is equal to the one added before
				continue;//skip it
			else
				ret.push_back( prop );
		}
	} else {
		LOG( Runtime, error ) << "Cannot get chunk-properties from non clean images. Run reIndex first";
	}

	return ret;
}

size_t Image::getBytesPerVoxel() const
{
	size_t bytes = chunkPtrAt( 0 )->bytesPerVoxel();
	BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
		LOG_IF( bytes != ref->bytesPerVoxel(), Debug, warning )
				<< "Not all voxels have the same byte size (" << bytes << "!=" << ref->bytesPerVoxel() << "). Using the biggest.";

		if( bytes < ref->bytesPerVoxel() ) {
			bytes = ref->bytesPerVoxel();
		}
	}
	return bytes;
}

std::pair<util::ValueReference, util::ValueReference> Image::getMinMax () const
{
	std::pair<util::ValueReference, util::ValueReference> ret;

	if( !lookup.empty() ) {
		std::vector<boost::shared_ptr<Chunk> >::const_iterator i = lookup.begin();
		ret = ( *i )->getMinMax();

		for( ++i; i != lookup.end(); ++i ) {
			std::pair<util::ValueReference, util::ValueReference> current = ( *i )->getMinMax();

			if( ret.first->gt( *current.first ) )
				ret.first = current.first;

			if( ret.second->lt( *current.second ) )
				ret.second = current.second;
		}
	}

	return ret;
}

std::pair< util::ValueReference, util::ValueReference > Image::getScalingTo( short unsigned int targetID, autoscaleOption scaleopt ) const
{
	LOG_IF( !clean, Debug, error ) << "You should run reIndex before running this";
	std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();

	BOOST_FOREACH( const boost::shared_ptr<const Chunk> &ref, lookup ) { //find a chunk which would be converted
		if( targetID != ref->getTypeID() ) {
			const scaling_pair scale = ref->getScalingTo( targetID, minmax, scaleopt );
			LOG_IF( scale.first.isEmpty() || scale.second.isEmpty(), Debug, error ) << "Returning an invalid scaling. This is bad!";
			return scale; // and ask that for the scaling
		}
	}
	return std::make_pair( //ok seems like no conversion is needed - return 1/0
			   util::ValueReference( util::Value<uint8_t>( 1 ) ),
			   util::ValueReference( util::Value<uint8_t>( 0 ) )
		   );
}

size_t Image::compare( const isis::data::Image &comp ) const
{
	size_t ret = 0;
	LOG_IF( ! ( clean && comp.clean ), Debug, error )
			<< "Comparing unindexed images will cause you trouble, run reIndex()!";

	if ( getSizeAsVector() != comp.getSizeAsVector() ) {
		LOG( Runtime, warning ) << "Size of images differs (" << getSizeAsVector() << "/"
								<< comp.getSizeAsVector() << "). Adding difference to the result.";
		ret += ( getSizeAsVector() - comp.getSizeAsVector() ).product();
	}

	util::ivector4 compVect( util::minVector( chunkPtrAt( 0 )->getSizeAsVector(), comp.chunkPtrAt( 0 )->getSizeAsVector() ) );
	util::ivector4 start;
	const size_t increment = compVect.product();

	for ( size_t i = 0; i < getVolume(); i += increment ) {
		const size_t nexti = i + increment - 1;
		const std::pair<size_t, size_t> c1pair1( i / chunkVolume, i % chunkVolume );
		const std::pair<size_t, size_t> c1pair2( nexti / chunkVolume, nexti % chunkVolume );
		const std::pair<size_t, size_t> c2pair1( i / comp.chunkVolume, i % comp.chunkVolume );
		assert( c1pair1.first == c1pair2.first );
		LOG( Debug, verbose_info ) << "Comparing chunks at " << c1pair1.first << " and "   << c2pair1.first;
		const Chunk &c1 = *chunkPtrAt( c1pair1.first );
		const Chunk &c2 = *( comp.chunkPtrAt( c2pair1.first ) );
		LOG( Debug, verbose_info )
				<< "Start positions are " << c1pair1.second << " and " << c2pair1.second
				<< " and the length is " << c1pair2.second - c1pair1.second;
		ret += c1.compareRange( c1pair1.second, c1pair2.second, c2, c2pair1.second );
	}

	return ret;
}

Image::orientation Image::getMainOrientation()const
{
	LOG_IF( ! isValid() || ! clean, Debug, warning ) << "You should not run this on non clean image. Run reIndex first.";
	util::fvector4 row = getPropertyAs<util::fvector4>( "rowVec" );
	util::fvector4 column = getPropertyAs<util::fvector4>( "columnVec" );
	row.norm();
	column.norm();
	LOG_IF( row.dot( column ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
	const util::fvector4 crossVec = util::fvector4(
										row[1] * column[2] - row[2] * column[1],
										row[2] * column[0] - row[0] * column[2],
										row[0] * column[1] - row[1] * column[0]
									);
	const util::fvector4 x( 1, 0 ), y( 0, 1 ), z( 0, 0, 1 );
	double a_axial    = std::acos( crossVec.dot( z ) ) / M_PI;
	double a_sagittal = std::acos( crossVec.dot( x ) ) / M_PI;
	double a_coronal  = std::acos( crossVec.dot( y ) ) / M_PI;
	bool a_inverse = false, s_inverse = false, c_inverse = false;
	LOG( Debug, info ) << "Angles to vectors are " << a_sagittal << " to x, " << a_coronal << " to y and " << a_axial << " to z";

	if( a_axial > .5 ) {
		a_axial = std::abs( a_axial - 1 );
		a_inverse = true;
	}

	if( a_sagittal > .5 ) {
		a_sagittal = std::abs( a_sagittal - 1 );
		s_inverse = true;
	}

	if( a_coronal > .5 ) {
		a_coronal = std::abs( a_coronal - 1 );
		c_inverse = true;
	}

	if( a_axial <= .25 )
		return a_inverse ? reversed_axial : axial;
	else if( a_sagittal <= .25 )
		return s_inverse ? reversed_sagittal : sagittal;
	else if( a_coronal <= .25 )
		return c_inverse ? reversed_coronal : coronal;
	else
		assert( false );

	return axial; //will never be reached
}

unsigned short Image::getMajorTypeID() const
{
	std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
	LOG( Debug, info ) << "Determining  datatype of image with the value range " << minmax;

	if( minmax.first->getTypeID() == minmax.second->getTypeID() ) { // ok min and max are the same type - trivial case
		return minmax.first->getTypeID() << 8; // btw: we do the shift, because min and max are Value - but we want the id's ValuePtr
	} else if( minmax.first->fitsInto( minmax.second->getTypeID() ) ) { // if min fits into the type of max, use that
		return minmax.second->getTypeID() << 8; //@todo maybe use a global static function here instead of a obscure shit operation
	} else if( minmax.second->fitsInto( minmax.first->getTypeID() ) ) { // if max fits into the type of min, use that
		return minmax.first->getTypeID() << 8;
	} else {
		LOG( Runtime, error ) << "Sorry I dont know which datatype I should use. (" << minmax.first->getTypeName() << " or " << minmax.second->getTypeName() << ")";
		std::stringstream o;
		o << "Type selection failed. Range was: " << minmax;
		throw( std::logic_error( o.str() ) );
		return std::numeric_limits<unsigned char>::max();
	}
}
std::string Image::getMajorTypeName() const
{
	return util::getTypeMap()[getMajorTypeID()];
}

bool Image::convertToType( short unsigned int ID )
{
	// get value range of the image for the conversion
	scaling_pair scale = getScalingTo( ID );

	LOG( Debug, info ) << "Computed scaling of the original image data: [" << scale << "]";
	bool retVal = true;
	//we want all chunks to be of type ID - so tell them
	BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, lookup ) {
		retVal &= ref->convertToType( ID, scale );
	}
	return retVal;
}

size_t Image::spliceDownTo( dimensions dim ) //rowDim = 0, columnDim, sliceDim, timeDim
{
	if( lookup[0]->getRelevantDims() < ( size_t ) dim ) {
		LOG( Debug, error ) << "The dimensionality of the chunks of this image is already below " << dim << " cannot splice it.";
		return 0;
	} else if( lookup[0]->getRelevantDims() == ( size_t ) dim ) {
		LOG( Debug, info ) << "Skipping useless splicing, relevantDims is allready " << lookup[0]->getRelevantDims();
		return lookup.size();
	}

	LOG_IF( lookup[0]->getRelevantDims() == ( size_t ) dim, Debug, info ) << "Running useless splice, the dimensionality of the chunks of this image is already " << dim;
	LOG_IF( hasProperty( "acquisitionTime" ) || lookup[0]->hasProperty( "acquisitionTime" ), Debug, warning ) << "Splicing images with acquisitionTime will cause you lots of trouble. You should remove that before.";
	util::FixedVector<size_t, 4> image_size = getSizeAsVector();

	for( int i = 0; i < dim; i++ )
		image_size[i] = 1;

	// get a list of needed properties (everything which is missing in a newly created chunk plus everything which is needed for autosplice)
	const std::list<util::PropertyMap::KeyType> splice_needed = util::stringToList<util::PropertyMap::KeyType>( util::PropertyMap::KeyType( "voxelSize,voxelGap,rowVec,columnVec,sliceVec,indexOrigin,acquisitionNumber" ), ',' );
	util::PropertyMap::KeyList needed = MemChunk<short>( 1 ).getMissing();
	needed.insert( splice_needed.begin(), splice_needed.end() );
	struct splicer {
		dimensions m_dim;
		Image &m_image;
		size_t m_amount;
		splicer( dimensions dimemsion, size_t amount, Image &image ): m_dim( dimemsion ), m_image( image ), m_amount( amount ) {}
		void operator()( const Chunk &ch ) {
			const size_t topDim = ch.getRelevantDims() - 1;

			if( topDim >= ( size_t ) m_dim ) { // ok we still have to splice that
				const size_t subSize = m_image.getSizeAsVector()[topDim];
				assert( !( m_amount % subSize ) ); // there must not be any "remaining"
				splicer sub( m_dim, m_amount / subSize, m_image );
				BOOST_FOREACH( const Chunk & ref, ch.autoSplice( uint32_t( m_amount / subSize ) ) ) {
					sub( ref );
				}
			} else { // seems like we're done - insert it into the image
				assert( ch.getRelevantDims() == ( size_t ) m_dim ); // index of the higest dim>1 (ch.getRelevantDims()-1) shall be equal to the dim below the requested splicing (m_dim-1)
				LOG(Debug,verbose_info) << "Inserting splice result of size " << ch.getSizeAsVector() << " at " << ch.propertyValue("indexOrigin");
				m_image.insertChunk( ch );
			}
		}
	};
	std::vector<boost::shared_ptr<Chunk> > buffer = lookup; // store the old lookup table
	lookup.clear();
	set.clear(); // clear the image, so we can insert the splices
	//static_cast<util::PropertyMap::base_type*>(this)->clear(); we can keep the common properties - they will be merged with thier own copies from the chunks on the next reIndex
	splicer splice( dim, image_size.product(), *this );
	BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, buffer ) {
		BOOST_FOREACH( const util::PropertyMap::KeyType & need, needed ) { //get back properties needed for the
			if( !ref->hasProperty( need ) && this->hasProperty( need ) ) {
				ref->propertyValue( need ) = this->propertyValue( need );
			}
		}
		splice( *ref );
	}
	reIndex();
	return lookup.size();
}

size_t Image::foreachChunk( ChunkOp &op, bool copyMetaData )
{
	size_t err = 0;
	checkMakeClean();
	util::FixedVector<size_t, 4> imgSize = getSizeAsVector();
	util::FixedVector<size_t, 4> chunkSize = getChunk( 0, 0, 0, 0 ).getSizeAsVector();
	util::FixedVector<size_t, 4> pos;

	for( pos[timeDim] = 0; pos[timeDim] < imgSize[timeDim]; pos[timeDim] += chunkSize[timeDim] ) {
		for( pos[sliceDim] = 0; pos[sliceDim] < imgSize[sliceDim]; pos[sliceDim] += chunkSize[sliceDim] ) {
			for( pos[columnDim] = 0; pos[columnDim] < imgSize[columnDim]; pos[columnDim] += chunkSize[columnDim] ) {
				for( pos[rowDim] = 0; pos[rowDim] < imgSize[rowDim]; pos[rowDim] += chunkSize[rowDim] ) {
					Chunk ch = getChunk( pos[rowDim], pos[columnDim], pos[sliceDim], pos[timeDim], copyMetaData );

					if( op( ch, pos ) == false )
						err++;
				}
			}
		}
	}

	return err;
}

size_t Image::getNrOfColumms() const
{
	return getDimSize( data::rowDim );
}

size_t Image::getNrOfRows() const
{
	return getDimSize( data::columnDim );
}
size_t Image::getNrOfSlices() const
{
	return getDimSize( data::sliceDim );
}
size_t Image::getNrOfTimesteps() const
{
	return getDimSize( data::timeDim );
}

util::fvector4 Image::getFoV() const
{
	util::fvector4 voxelGap;

	if ( hasProperty( "voxelGap" ) ) {
		voxelGap = getPropertyAs<util::fvector4>( "voxelGap" );

		for ( size_t i = 0; i < dims; i++ )
			if ( voxelGap[i] == -std::numeric_limits<float>::infinity() ) {
				LOG( Runtime, info ) << "Ignoring unknown voxel gap in direction " << i;
				voxelGap[i] = 0;
			}
	}

	return _internal::NDimensional<4>::getFoV( getPropertyAs<util::fvector4>( "voxelSize" ), voxelGap );
}


} // END namespace data
} // END namespace isis
