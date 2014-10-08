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
namespace _internal {
struct splicer {
	dimensions m_dim;
	Image &m_image;
	size_t m_amount;
	splicer( dimensions dimemsion, size_t amount, Image &image ): m_dim( dimemsion ), m_image( image ), m_amount( amount ) {}
	void operator()( boost::shared_ptr<Chunk> &ptr ) {operator()(*ptr);}
	void operator()( Chunk &ch ) {
		const size_t topDim = ch.getRelevantDims() - 1;

		if( topDim >= ( size_t ) m_dim ) { // ok we still have to splice that
			const size_t subSize = m_image.getSizeAsVector()[topDim];
			assert( !( m_amount % subSize ) ); // there must not be any "remaining"
			splicer sub( m_dim, m_amount / subSize, m_image );
			BOOST_FOREACH( Chunk ref, ch.autoSplice( uint32_t( m_amount / subSize ) ) ) {
				sub( ref );
			}
		} else { // seems like we're done - insert it into the image
			assert( ch.getRelevantDims() == ( size_t ) m_dim ); // index of the higest dim>1 (ch.getRelevantDims()-1) shall be equal to the dim below the requested splicing (m_dim-1)
			LOG( Debug, verbose_info ) << "Inserting splice result of size " << ch.getSizeAsVector() << " at " << ch.property( "indexOrigin" );
			m_image.insertChunk( ch );
		}
	}
};
}

ChunkOp::~ChunkOp() {}

Image::Image ( ) : set( defaultChunkEqualitySet ), clean( false )
{
	util::Singletons::get<NeededsList<Image>, 0>().applyTo( *this );
	set.addSecondarySort( "acquisitionNumber" );
}

Image::Image ( const Chunk &chunk, dimensions min_dim ) :
	_internal::NDimensional<4>(), util::PropertyMap(), minIndexingDim( min_dim ), set( defaultChunkEqualitySet ), clean( false )
{
	util::Singletons::get<NeededsList<Image>, 0>().applyTo( *this );
	set.addSecondarySort( "acquisitionNumber" );
	
	if ( ! ( insertChunk( chunk ) && reIndex() && isClean() ) ) {
		LOG( Runtime, error ) << "Failed to create image from single chunk.";
	} else if( !isValid() ) {
		LOG_IF( !getMissing().empty(), Debug, warning )
				<< "The created image is missing some properties: " << getMissing() << ". It will be invalid.";
	}
}

Image::Image( const data::Image &ref ): _internal::NDimensional<4>(), util::PropertyMap(),
	set( "" )/*SortedChunkList has no default constructor - lets just make an empty (and invalid) set*/
{
	( *this ) = ref; // set will be replaced here anyway
}

Image &Image::operator=( const data::Image &ref )
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

void Image::swapDim( short unsigned int dim_a, short unsigned int dim_b )
{
	_internal::NDimensional<4>::swapDim( dim_a, dim_b, begin() );
}


void Image::deduplicateProperties()
{
	LOG_IF( lookup.empty(), Debug, error ) << "The lookup table is empty. Won't do anything.";
	if(lookup.empty())
		return;

	util::PropertyMap common=*lookup.front();  //copy all props from the first chunk

	// common now has all props (from the first chunk)
	for(size_t i=1;i<lookup.size();i++)
		lookup[i]->removeUncommon( common );//remove everything which isn't common from common
	//then remove remaining common props from the chunks
	BOOST_FOREACH ( const boost::shared_ptr<Chunk> &p, lookup ) 
		p->remove( common, false ); //this _won't_ keep needed properties - so from here on the chunks of the image are invalid
		
	const PathSet rej = this->join(common);
	LOG_IF(!rej.empty(),Debug,error) << "Some props where rejected when joining the chunk's commons into the image (" << rej << ")";

	LOG( Debug, verbose_info ) << "common properties now in the image: " << common;
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
		LOG( Runtime, warning ) << "Inserting into already indexed images is inefficient. You should not do that.";

		// re-move all properties from the image back into the chunks 
		BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, lookup ) {
			ref->transfer( *this );
		}
	}


	if( set.insert( chunk ) ) { // if the insertion was successful the image has to be reindexed anyway
		clean = false;
		lookup.clear();
		return true;
	} else {
		// if the insertion failed, but the image was clean - de-duplicate properties again
		// the image is still clean - no need to reindex
		#warning test me (why deduplicate)
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

bool Image::transformCoords( boost::numeric::ublas::matrix< float > transform_matrix, bool transformCenterIsImageCenter )
{
#warning test me
	//for transforming we have to ensure to have the below properties in our chunks and image
	static const char  *neededProps[] = {"indexOrigin", "rowVec", "columnVec", "sliceVec", "voxelSize"};
	//propagate needed properties to chunks
	std::set<PropPath> propPathList;
	BOOST_FOREACH ( const char * prop, neededProps ) {
		const util::PropertyMap::PropPath pPath( prop );

		if( hasProperty ( pPath ) ) {
			const util::fvector3 p = getValueAs<util::fvector3> ( pPath );
			BOOST_FOREACH ( std::vector<boost::shared_ptr< data::Chunk> >::reference chRef, lookup ) {
				if ( !chRef->hasProperty ( pPath ) ) {
					chRef->setValueAs<util::fvector3> ( pPath, p );
					propPathList.insert( pPath );
				}
			}
		} else {
			LOG( Runtime, error ) << "Cannot do transformCoords on image without " << prop;
			return false;
		}
	}

	BOOST_FOREACH ( std::vector<boost::shared_ptr< data::Chunk> >::reference chRef, lookup ) {
		if ( !chRef->transformCoords ( transform_matrix, transformCenterIsImageCenter ) ) {
			return false;
		}

		BOOST_FOREACH( std::list<PropPath>::const_reference pPathNotNeeded, propPathList ) {
			chRef->remove( pPathNotNeeded );
		}
	}
	//      establish initial state

	if ( !isis::data::_internal::transformCoords ( *this, getSizeAsVector(), transform_matrix, transformCenterIsImageCenter ) ) {
		LOG ( Runtime, error ) << "Error during transforming the coords of the image.";
		return false;
	}

	return true;
}

dimensions Image::mapScannerAxisToImageDimension( scannerAxis scannerAxes )
{
#warning test me
	boost::numeric::ublas::matrix<float> latchedOrientation = boost::numeric::ublas::zero_matrix<float>( 4, 4 );
	boost::numeric::ublas::vector<float>mapping( 4 );
	latchedOrientation( getValueAs<util::fvector3>("rowVec").getBiggestVecElemAbs(), 0 ) = 1;
	latchedOrientation( getValueAs<util::fvector3>("columnVec").getBiggestVecElemAbs(), 1 ) = 1;
	latchedOrientation( getValueAs<util::fvector3>("sliceVec").getBiggestVecElemAbs(), 2 ) = 1;
	latchedOrientation( 3, 3 ) = 1;

	for( size_t i = 0; i < 4; i++ ) {
		mapping( i ) = i;
	}

	return static_cast<dimensions>( boost::numeric::ublas::prod( latchedOrientation, mapping )( scannerAxes ) );

}


bool Image::reIndex(optional< util::slist& > rejected)
{
	if ( set.isEmpty() ) {
		LOG( Debug, warning ) << "Reindexing an empty image is useless.";
		return false;
	}

	if( set.makeRectangular(rejected) > 0 && set.isEmpty() ) {
		LOG( Runtime, error ) << "No retangular image data left. Skipping";
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

	//move all props from the image and move them (back) into the chunks to redo common check
	for(int i=0;i<lookup.size()-1;i++) // copy the first n-1
		lookup[i]->join(*this); 
	lookup.back()->transfer(*this); //transfer into the last
	
	//clean generated props if they're still here (which would mean the chunks has their own -- which is a good thing)
	if(hasProperty("indexOrigin"))remove( "indexOrigin" );
	if(hasProperty("rowVec"))remove( "rowVec" );
	if(hasProperty("columnVec"))remove( "columnVec" );
	if(hasProperty("sliceVec"))remove( "sliceVec" );

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Determine structure of the image by searching for dimensional breaks in the chunklist
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
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
	const util::PropertyMap::key_type vectors[] = {"rowVec", "columnVec", "sliceVec"};
	int oneCnt = 0;

	BOOST_FOREACH( const util::PropertyMap::key_type & ref, vectors ) {
		const optional< util::fvector3& > found=refValueAs<util::fvector3>( ref );
		if ( found ) {
			util::fvector3 &vec = found.get();

			if( vec.sqlen() == 0 ) {
				util::fvector3  v_one;
				v_one[oneCnt] = 1;
				LOG( Runtime, error )
						<< "The existing " << ref << " " << vec << ( hasProperty( "source" ) ? " from " + getValueAs<std::string>( "source" ) : "" ) << " has the length zero. Falling back to " << v_one << ".";
				vec = v_one;
			}

			vec.norm();
		}
		oneCnt++;
	}

	if(!hasProperty("indexOrigin")){ // if there is no common indexOrigin
		optional< const util::PropertyValue& > found =first.hasProperty("indexOrigin");
		if(found) // get it from the first chunk - which than by definition should have one
			touchProperty("indexOrigin")=found.get();
		else{
			LOG(Runtime,error) << "No indexOrigin found " << " falling back to " << util::fvector3();
			setValueAs("indexOrigin",util::fvector3());
		}
	}

	// check voxelsize
	util::fvector3 &voxeSize = refValueAs<util::fvector3>( "voxelSize" ).get();
	for( int i = 0; i < 3; i++ ) {
		if( voxeSize[i] == 0 || std::isinf( voxeSize[i] ) ) {
			LOG( Runtime, warning ) << "voxelSize[" << i << "]=="  << voxeSize[i] << " is invalid, using 1";
			voxeSize[i] = 1;
		}
	}


	//if we have at least two slides (and have slides (with different positions) at all)
	const optional< const util::PropertyValue& > firstV = first.hasProperty( "indexOrigin" );

	if ( chunk_dims == 2 && structure_size[2] > 1 && firstV ) {
		const Chunk &last = chunkAt( structure_size[2] - 1 );

		optional< const util::PropertyValue& > lastV = last.hasProperty( "indexOrigin" );

		if ( lastV ) {
			//check the slice vector
			util::fvector3 distVecNorm = lastV->as<util::fvector3>() - firstV->as<util::fvector3>();
			LOG_IF( distVecNorm.len() == 0, Runtime, error )
					<< "The distance between the the first and the last chunk is zero. Thats bad, because I'm going to normalize it.";
			distVecNorm.norm();

			const optional< util::fvector3& > sliceVec = refValueAs<util::fvector3>( "sliceVec" );

			if ( sliceVec ) {
				LOG_IF( ! distVecNorm.fuzzyEqual( *sliceVec ), Runtime, info )
						<< "The existing sliceVec " << sliceVec
						<< " differs from the distance vector between chunk 0 and " << structure_size[2] - 1
						<< " " << distVecNorm;
			} else {
				LOG( Debug, info )
						<< "used the distance between chunk 0 and " << structure_size[2] - 1
						<< " to synthesize the missing sliceVec as " << distVecNorm;
				setValueAs( "sliceVec", distVecNorm); // this should message if there really is a conflict
			}

			const float avDist = ( lastV->as<util::fvector3>() - firstV->as<util::fvector3>() ).len() / ( structure_size[2] - 1 ); //average dist between the middle of two slices

			const float sliceDist = avDist - voxeSize[2]; // the gap between two slices

			if ( sliceDist > 0 ) {
				static const float inf = -std::numeric_limits<float>::infinity();

				util::fvector3 &voxelGap = refValueAsOr( "voxelGap", util::fvector3( 0, 0, inf ) ).get(); //if there is no voxelGap yet, we create it as (0,0,inf)

				if ( voxelGap[2] != inf ) {
					LOG_IF( ! util::fuzzyEqual( voxelGap[2], sliceDist, 50 ), Runtime, warning )
							<< "The existing slice distance (voxelGap[2]) " << util::MSubject( voxelGap[2] )
							<< " differs from the distance between chunk 0 and 1, which is " << sliceDist;
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
	const optional< util::fvector3& > rrow = refValueAs<util::fvector3>( "rowVec" ),rcolumn = refValueAs<util::fvector3>( "columnVec" );
	if(rrow && rcolumn){
		const util::fvector3 &row=*rrow,&column=*rcolumn;
		LOG_IF( row.dot( column ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
		const util::fvector3 crossVec = util::fvector3( //we could use their cross-product as sliceVector
											row[1] * column[2] - row[2] * column[1],
											row[2] * column[0] - row[0] * column[2],
											row[0] * column[1] - row[1] * column[0]
										);
		optional< util::fvector3& > sliceVec = refValueAs<util::fvector3>( "sliceVec" );

		if ( sliceVec ) {
			LOG_IF( std::acos( crossVec.dot( *sliceVec ) )  > 180 / M_PI, Runtime, warning ) //angle more than one degree
					<< "The existing sliceVec " << sliceVec << " differs from the cross product of the row- and column vector " << crossVec;
		} else {
			// We dont know anything about the slice-direction
			// we just guess its along the positive cross-product between row- and column direction
			// so at least warn the user if we do that long shot
			LOG( Runtime, info )
					<< "used the cross product between rowVec and columnVec as sliceVec:"
					<< crossVec << ". That might be wrong!";
			setValueAs( "sliceVec", crossVec );
		}
	} else {
		LOG(Runtime,warning) << "Image has no common rowVec/columnVec, won't check for sliceVec";
	}


	optional< util::fvector3 & > storedFoV = refValueAs<util::fvector3>( "fov" );

	if ( storedFoV ) {
		const util::fvector3 calcFoV = getFoV();

		bool ok = true;

		for ( size_t i = 0; i < 3; i++ ) {
			if ( ( *storedFoV )[i] != -std::numeric_limits<float>::infinity() ) {
				ok &= util::fuzzyEqual( ( *storedFoV )[i], calcFoV[i] );
			} else
				( *storedFoV )[i] = calcFoV[i];
		}

		LOG_IF( ! ok, Runtime, info )
				<< "The calculated field of view differs from the stored " << util::MSubject( *storedFoV ) << "/" << calcFoV;
	}

	LOG_IF( ! isValid(), Runtime, warning ) << "The image is not valid after reindexing. Missing properties: " << getMissing();

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

void Image::copyToValueArray( ValueArrayBase &dst, scaling_pair scaling ) const
{
	if( getVolume() > dst.getLength() ) {
		LOG( Runtime, error ) << "Image wont fit into the ValueArray, wont copy..";
		return;
	}

	if ( clean ) {
		if ( scaling.first.isEmpty() || scaling.second.isEmpty() ) {
			scaling = getScalingTo ( dst.getTypeID() );
		}

		std::vector< ValueArrayReference > targets;

		if( lookup.size() > 1 ) { //if there are more than 1 chunks
			//splice target to have the same parts as the image
			targets = dst.splice( lookup.front()->getVolume() );
		} else {
			//just put that ValueArray into the list
			targets.push_back( dst );
		}

		std::vector< ValueArrayReference >::iterator target = targets.begin();
		BOOST_FOREACH ( const boost::shared_ptr<Chunk> &ref, lookup ) { // copy chunks into the parts
			if ( !ref->getValueArrayBase().copyTo ( **target, scaling ) ) {
				LOG ( Runtime, error )
						<< "Failed to copy raw data of type " << ref->getTypeName() << " from " << getSizeAsString() << "-image into ValueArray of type "
						<< dst.getTypeName() << " and length " << dst.getLength();
			}

			target++;
		}
	} else {
		LOG ( Runtime, error ) << "Cannot copy from non clean images. Run reIndex first";
	}
}

Image Image::copyByID( short unsigned int ID, scaling_pair scaling ) const
{
	Image ret( *this ); // ok we just cheap-copied the whole image

	//we want deep copies of the chunks, and we want them to be of type ID
	struct : _internal::SortedChunkList::chunkPtrOperator {
		std::pair<util::ValueReference, util::ValueReference> scale;
		unsigned short ID;
		boost::shared_ptr<Chunk> operator() ( const boost::shared_ptr< Chunk >& ptr ) {
			return boost::shared_ptr<Chunk> ( new Chunk ( ptr->copyByID( ID, scale ) ) );
		}
	} conv_op;

	if( ID && ( scaling.first.isEmpty() || scaling.second.isEmpty() ) ) // if we have an ID but no scaling, compute it
		conv_op.scale = getScalingTo( ID );

	conv_op.ID = ID;

	ret.set.transform ( conv_op );

	if ( ret.isClean() ) {
		ret.lookup = ret.set.getLookup(); // the lookup table still points to the old chunks
	} else {
		LOG ( Debug, info ) << "Copied unclean image. Running reIndex on the copy.";
		ret.reIndex();
	}

	return *this;

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
		const util::fvector3 firstV = chunkAt( 0 ).getValueAs<util::fvector3>( "indexOrigin" );
		const util::fvector3 secondV = chunkAt( base_stride ).getValueAs<util::fvector3>( "indexOrigin" );
		const util::fvector3 dist1 = secondV - firstV;

		if( dist1.sqlen() == 0 ) { //if there is no geometric structure anymore - so asume its flat from here on
			LOG( Debug, info ) << "Distance between 0 and " << util::MSubject( base_stride )
							   << " is zero. Assuming there are no dimensional breaks anymore. Returning " << util::MSubject( base_stride );
			return base_stride;
		} else for ( size_t i = base_stride; i < lookup.size() - base_stride; i += base_stride ) {  // compare every follwing distance to that
				const util::fvector3 thisV = chunkAt( i ).getValueAs<util::fvector3>( "indexOrigin" );
				const util::fvector3 nextV = chunkAt( i + base_stride ).getValueAs<util::fvector3>( "indexOrigin" );
				const util::fvector3 distFirst = nextV - firstV;
				const util::fvector3 distThis = nextV - thisV;
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

std::list<util::PropertyValue> Image::getChunksProperties( const util::PropertyMap::key_type &key, bool unique )const
{
	std::list<util::PropertyValue > ret;

	if( clean ) {
		BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
			const optional< const util::PropertyValue& > prop = boost::const_pointer_cast<const Chunk>(ref)->hasProperty( key );

			if(unique){ // if unique
				if( (prop && !ret.empty() &&  prop->eq(ret.back())) || // if there is prop, skip if its equal
					!prop //if there is none skip anyway
				) 
					continue;
			}
			ret.push_back( prop.get_value_or(util::PropertyValue()) );
		}
	} else {
		LOG( Runtime, error ) << "Cannot get chunk-properties from non clean images. Run reIndex first";
	}

	return ret;
}

size_t Image::getMaxBytesPerVoxel() const
{
	size_t bytes = chunkPtrAt( 0 )->getBytesPerVoxel();
	BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
		LOG_IF( bytes != ref->getBytesPerVoxel(), Debug, warning )
				<< "Not all voxels have the same byte size (" << bytes << "!=" << ref->getBytesPerVoxel() << "). Using the biggest.";

		if( bytes < ref->getBytesPerVoxel() ) {
			bytes = ref->getBytesPerVoxel();
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

// @todo this wont work with images of more 2 two different data types
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
		ret += c1.getValueArrayBase().compare( c1pair1.second, c1pair2.second, c2.getValueArrayBase(), c2pair1.second );
	}

	return ret;
}

Image::orientation Image::getMainOrientation()const
{
	LOG_IF( ! isValid() || ! clean, Debug, warning ) << "You should not run this on non clean image. Run reIndex first.";
	util::fvector3 row = getValueAs<util::fvector3>( "rowVec" );
	util::fvector3 column = getValueAs<util::fvector3>( "columnVec" );
	row.norm();
	column.norm();
	LOG_IF( row.dot( column ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
	const util::fvector3 crossVec = util::fvector3(
										row[1] * column[2] - row[2] * column[1],
										row[2] * column[0] - row[0] * column[2],
										row[0] * column[1] - row[1] * column[0]
									);
	const util::fvector3 x( 1, 0 ), y( 0, 1 ), z( 0, 0, 1 );
	double a_axial    = std::acos( crossVec.dot( z ) ) / M_PI;
	double a_sagittal = std::acos( crossVec.dot( x ) ) / M_PI;
	double a_coronal  = std::acos( crossVec.dot( y ) ) / M_PI;
	bool a_inverse = false, s_inverse = false, c_inverse = false;
	LOG( Debug, info ) << "Angles to vectors are " << ( a_sagittal * 180 ) << " to x, " << ( a_coronal * 180 ) << " to y and " << ( a_axial * 180 ) << " to z";

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
	switch( getChunk( 0 ).getTypeID() ) { // dont do smart typeID detection for types who cant do minmax
	case data::ValueArray<util::color24>::staticID:
	case data::ValueArray<util::color48>::staticID:
	case data::ValueArray<std::complex< float >  >::staticID:
	case data::ValueArray<std::complex< double > >::staticID:
		LOG( Debug, info ) << "Using flat typeID for " << getChunk( 0 ).getTypeName() << " because I cannot compute min/max";
		return getChunk( 0 ).getTypeID();
		break;
	default:
		std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
		LOG( Debug, info ) << "Determining  datatype of image with the value range " << minmax;

		if( minmax.first->getTypeID() == minmax.second->getTypeID() ) { // ok min and max are the same type - trivial case
			return minmax.first->getTypeID() << 8; // btw: we do the shift, because min and max are Value - but we want the ID's ValueArray
		} else if( minmax.first->fitsInto( minmax.second->getTypeID() ) ) { // if min fits into the type of max, use that
			return minmax.second->getTypeID() << 8; //@todo maybe use a global static function here instead of a obscure shit operation
		} else if( minmax.second->fitsInto( minmax.first->getTypeID() ) ) { // if max fits into the type of min, use that
			return minmax.first->getTypeID() << 8;
		} else {
			LOG( Runtime, error ) << "Sorry I dont know which datatype I should use. (" << minmax.first->getTypeName() << " or " << minmax.second->getTypeName() << ")";
			std::stringstream o;
			o << "Type selection failed. Range was: " << minmax;
			throw( std::logic_error( o.str() ) );
		}

		break;
	}

	return 0; // id 0 is invalid
}
std::string Image::getMajorTypeName() const
{
	return util::getTypeMap()[getMajorTypeID()];
}

bool Image::convertToType( short unsigned int ID, autoscaleOption scaleopt )
{
	bool retVal = true;
	BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, lookup ) {
		retVal &= ( ref->getTypeID() == ID );
	}

	if( retVal ) // if all chunks allready have the requested type we can skip the rest
		return true;

	// get value range of the image for the conversion
	scaling_pair scale = getScalingTo( ID, scaleopt );

	LOG( Debug, info ) << "Computed scaling of the original image data: [" << scale << "]";
	retVal = true;
	//we want all chunks to be of type ID - so tell them
	BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, lookup ) {
		retVal &= ref->convertToType( ID, scale );
	}
	return retVal;
}

size_t Image::spliceDownTo( dimensions dim )   //rowDim = 0, columnDim, sliceDim, timeDim
{
	if( lookup[0]->getRelevantDims() < ( size_t ) dim ) {
		LOG( Debug, error ) << "The dimensionality of the chunks of this image is already below " << dim << " cannot splice it.";
		return 0;
	} else if( lookup[0]->getRelevantDims() == ( size_t ) dim ) {
		LOG( Debug, info ) << "Skipping useless splicing, relevantDims is already " << lookup[0]->getRelevantDims();
		return lookup.size();
	}

	util::vector4<size_t> image_size = getSizeAsVector();

	for( int i = 0; i < dim; i++ )
		image_size[i] = 1;

	// get a list of needed properties (everything which is missing in a newly created chunk plus everything which is needed for autosplice)
	const std::list<util::PropertyMap::key_type> splice_needed = util::stringToList<util::PropertyMap::key_type>( util::PropertyMap::key_type( "voxelSize,voxelGap,rowVec,columnVec,sliceVec,indexOrigin,acquisitionNumber" ), ',' );
	util::PropertyMap::PathSet needed = MemChunk<short>( 1 ).getMissing();
	needed.insert( splice_needed.begin(), splice_needed.end() );
	// reset the Chunk set, so we can insert new splices
	set = _internal::SortedChunkList( defaultChunkEqualitySet );
	set.addSecondarySort( "acquisitionNumber" );

	clean = false; // mark the image for reIndexing

	//splice existing lists into the current chunks -- they will be spliced further if neccessary
	PropertyMap::splice(lookup.begin(),lookup.end(),true);
	
	//transfer properties needed for the chunk back into the chunk (if they're there but not lists)
	BOOST_FOREACH( util::PropertyMap::PathSet::const_reference need,  needed ) { 
		const boost::optional< util::PropertyValue& > foundNeed=this->hasProperty( need );
		if(foundNeed){
			BOOST_FOREACH( boost::shared_ptr<Chunk> &ref,  lookup ) {
				if( !ref->hasProperty( need ) ) {
					LOG( Debug, verbose_info ) << "Copying " << std::make_pair(need, *foundNeed) << " from the image to the chunk for splicing";
					ref->touchProperty( need ) = *foundNeed;
				} else 
					LOG(Debug,error) << need << " was found in the chunk although it is in the image as well. It will be deleted in the image";
			}
			this->remove(need);
		}
	}
	// do the splicing
	std::vector<boost::shared_ptr<Chunk> > buffer;
	buffer.swap(lookup); // move the old lookup table into a buffer, so its empty when the splicer starts inserting the new chunks
	std::for_each(buffer.begin(),buffer.end(),_internal::splicer( dim, image_size.product(), *this ));
	reIndex();
	return lookup.size();
}

size_t Image::foreachChunk( ChunkOp &op, bool copyMetaData )
{
	size_t err = 0;

	if( checkMakeClean() ) {
		util::vector4<size_t> imgSize = getSizeAsVector();
		util::vector4<size_t> chunkSize = getChunk( 0, 0, 0, 0 ).getSizeAsVector();
		util::vector4<size_t> pos;

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
	}

	return err;
}

size_t Image::getNrOfColumns() const
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

util::fvector3 Image::getFoV() const
{
	util::fvector4 voxelGap;

	if ( hasProperty( "voxelGap" ) ) {
		voxelGap = getValueAs<util::fvector4>( "voxelGap" );

		for ( size_t i = 0; i < 3; i++ )
			if ( voxelGap[i] == -std::numeric_limits<float>::infinity() ) {
				LOG( Runtime, info ) << "Ignoring unknown voxel gap in direction " << i;
				voxelGap[i] = 0;
			}
	}

	const util::fvector4 ret = _internal::NDimensional<4>::getFoV( getValueAs<util::fvector4>( "voxelSize" ), voxelGap );

	LOG_IF( ret[timeDim], Runtime, warning ) << "Ignoring fourth dim extend of " << ret[timeDim] << " in Image";

	return util::fvector3( ret[0], ret[1], ret[2] );
}

Image::iterator Image::begin()
{
	if( checkMakeClean() ) {
		boost::shared_array<Chunk *> vec( new Chunk*[lookup.size()] );

		for( size_t i = 0; i < lookup.size(); i++ )
			vec[i] = lookup[i].get();

		return iterator( vec, lookup.size() );
	} else {
		LOG( Debug, error )  << "Image is not clean. Returning empty iterator ...";
		return iterator();
	}
}
Image::iterator Image::end() {return begin() + getVolume();}
Image::const_iterator Image::begin()const
{
	if( isClean() ) {
		boost::shared_array<Chunk *> vec( new Chunk*[lookup.size()] );

		for( size_t i = 0; i < lookup.size(); i++ )
			vec[i] = lookup[i].get();

		return const_iterator( vec, lookup.size() );
	} else {
		LOG( Debug, error )  << "Image is not clean. Returning empty iterator ...";
		return const_iterator();
	}
}
Image::const_iterator Image::end()const {return begin() + getVolume();}

const util::ValueReference Image::getVoxelValue ( size_t nrOfColumns, size_t nrOfRows, size_t nrOfSlices, size_t nrOfTimesteps ) const
{
	const size_t idx[] = {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps};
	LOG_IF( !isInRange( idx ), Debug, isis::error )
			<< "Index " << util::vector4<size_t>( idx ) << " is out of range (" << getSizeAsString() << ")";
	return begin()[getLinearIndex( idx )];
}
void Image::setVoxelValue ( const util::ValueReference &val, size_t nrOfColumns, size_t nrOfRows, size_t nrOfSlices, size_t nrOfTimesteps )
{
	const size_t idx[] = {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps};
	LOG_IF( !isInRange( idx ), Debug, isis::error )
			<< "Index " << util::vector4<size_t>( idx ) << " is out of range (" << getSizeAsString() << ")";
	begin()[getLinearIndex( idx )] = val;
}

boost::filesystem::path Image::getCommonSource()const
{
	boost::optional< const util::PropertyValue& > found=hasProperty( "source" );
	if( found )
		return found->as<std::string>();
	else
		return set.getCommonSource();
}

std::string Image::identify ( bool withpath, bool withdate )const
{
	_internal::SortedChunkList::getproplist seqNum("sequenceNumber"),seqDesc("sequenceDescription"),seqStart("sequenceStart");
	seqNum(*this);seqDesc(*this);seqStart(*this);
	return set.identify(withpath,withdate,seqNum,seqDesc,seqStart);
}


} // END namespace data
} // END namespace isis
