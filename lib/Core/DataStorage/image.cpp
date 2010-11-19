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

#include "DataStorage/image.hpp"
#include "CoreUtils/vector.hpp"
#include <boost/foreach.hpp>
#include "CoreUtils/property.hpp"
#include<boost/token_iterator.hpp>

#define _USE_MATH_DEFINES 1
#include <math.h>
#include <cmath>

namespace isis
{
namespace data
{

Image::Image ( ) : set( "indexOrigin", "sequenceNumber,readVec,phaseVec,sliceVec,coilChannelMask,DICOM/EchoNumbers" ), clean( false )
{
	addNeededFromString( needed );
	set.addSecondarySort( "acquisitionNumber" );
	set.addSecondarySort( "acquisitionTime" );
}

Image::Image( const isis::data::Image &ref ): set( "", "" )/*SortedChunkList has no default constructor - lets just make an empty (and invalid) set*/
{
	( *this ) = ref; // set will be replaced here anyway
}

Image &Image::operator=( const isis::data::Image &ref )
{
	//deep copy bases
	static_cast<util::PropMap &>( *this ) = static_cast<const util::PropMap &>( ref );
	static_cast<_internal::NDimensional< 4 >&>( *this ) = static_cast<const _internal::NDimensional< 4 >&>( ref );
	//deep copy members
	chunkVolume = ref.chunkVolume;
	clean = ref.clean;
	set = ref.set;
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
		LOG( Debug, info )	<< "Image is not clean. Running reIndex ...";

		if( !reIndex() ) {
			LOG( Runtime, error ) << "Reindexing failed -- undefined behavior ahead ...";
		}
	}

	return clean;
}


bool Image::insertChunk ( const Chunk &chunk )
{
	if ( chunk.volume() == 0 ) {
		LOG( Runtime, error )
				<< "Cannot insert empty Chunk (Size is " << chunk.getSizeAsString() << ").";
		return false;
	}

	if ( ! chunk.valid() ) {
		LOG( Runtime, error )
				<< "Cannot insert invalid chunk. Missing properties: " << chunk.getMissing();
		return false;
	}

	LOG_IF( chunk.getProperty<util::fvector4>( "indexOrigin" )[3] != 0, Debug, warning )
			<< " inserting chunk with nonzero at the 4th position - you shouldn't use the fourth dim for the time (use acquisitionTime)";

	if ( set.insert( chunk ) ) {
		clean = false;
		lookup.clear();
		return true;
	} else
		return false;
}


bool Image::reIndex()
{
	if ( set.empty() ) {
		LOG( Debug, warning ) << "Reindexing an empty image is useless.";
		return false;
	}

	if( !set.isRectangular() ) {
		LOG( Runtime, error ) << "The image is incomplete. Aborting reindex.";
		return false;
	}

	//redo lookup table
	lookup = set.getLookup();
	const size_t chunks = lookup.size();
	util::FixedVector<size_t, dims> size; //storage for the size of the chunk structure
	size.fill( 1 );
	//get primary attributes from geometrically first chunk - will be usefull
	const Chunk &first = chunkAt( 0 );
	const unsigned short chunk_dims = first.relevantDims();
	chunkVolume = first.volume();
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Determine structure of the image by searching for dimensional breaks in the chunklist
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//get indexOrigin from the geometrically first chunk
	propertyValue( "indexOrigin" ) = first.propertyValue( "indexOrigin" );
	//if there are many chunks, they must leave at least on dimension to the image to "sort" them in
	const size_t timesteps=set.getHorizontalSize();
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
			size[i] = getChunkStride( size.product() ) / size.product();
			assert( size[i] != 0 );
		}
	}
	if (sortDims < dims) {//if there is a timedim (not all dims was used for geometric sort)
		assert(size[sortDims]==1);
		size[sortDims]=timesteps; // fill the dim above the top geometric dim with the timesteps
	}
	assert( size.product() == lookup.size() );
	//Clean up the properties
	//@todo might fail if the image contains a prop that differs to that in the Chunks (which is equal in the chunks)
	util::PropMap common;
	std::set<std::string> uniques;
	first.toCommonUnique( common, uniques, true );

	for ( size_t i = 1; i < chunks; i++ ) {
		chunkAt( i ).toCommonUnique( common, uniques, false );
	}

	LOG( Debug, info ) << uniques.size() << " Chunk-unique properties found in the Image";
	LOG_IF( uniques.size(), Debug, verbose_info ) << util::list2string( uniques.begin(), uniques.end(), ", " );
	join( common );
	LOG_IF( ! common.empty(), Debug, verbose_info ) << "common properties saved into the image " << common;

	//remove common props from the chunks
	for ( size_t i = 0; i != lookup.size(); i++ )
		chunkAt( i ).remove( common, false ); //this _won't keep needed properties - so from here on the chunks of the image are invalid

	LOG_IF( ! common.empty(), Debug, verbose_info ) << "common properties removed from " << chunks << " chunks: " << common;

	// add the chunk-size to the image-size
	for ( unsigned short i = 0; i < chunk_dims; i++ )
		size[i] = first.dimSize( i );

	init( size ); // set size of the image
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//reconstruct some redundant information, if its missing
	//////////////////////////////////////////////////////////////////////////////////////////////////
	const std::string vectors[] = {"readVec", "phaseVec", "sliceVec"};
	BOOST_FOREACH( const std::string & ref, vectors ) {
		if ( hasProperty( ref ) ) {
			util::PropertyValue &prop = propertyValue( ref );
			LOG_IF( !prop->is<util::fvector4>(), Debug, error ) << "Using " << prop->typeName() << " as " << util::Type<util::fvector4>::staticName();
			util::fvector4 &vec = prop->castTo<util::fvector4>();
			LOG_IF( vec.len() == 0, Runtime, error )
					<< "The existing " << ref << " " << vec << " has the length zero. Thats bad, because I'm going to normalize it.";
			vec.norm();
		}
	}

	//if we have at least two slides (and have slides (with different positions) at all)
	if ( chunk_dims == 2 && size[2] > 1 && first.hasProperty( "indexOrigin" ) ) {
		const util::fvector4 thisV = first.getProperty<util::fvector4>( "indexOrigin" );
		const Chunk &last = chunkAt( size[2] - 1 );

		if ( last.hasProperty( "indexOrigin" ) ) {
			const util::fvector4 lastV = last.getProperty<util::fvector4>( "indexOrigin" );
			//check the slice vector
			util::fvector4 distVecNorm = lastV - thisV;
			LOG_IF( distVecNorm.len() == 0, Runtime, error )
					<< "The distance between the the first and the last chunk is zero. Thats bad, because I'm going to normalize it.";
			distVecNorm.norm();

			if ( hasProperty( "sliceVec" ) ) {
				const util::fvector4 sliceVec = getProperty<util::fvector4>( "sliceVec" );
				LOG_IF( ! distVecNorm.fuzzyEqual( sliceVec ), Runtime, warning )
						<< "The existing sliceVec " << sliceVec
						<< " differs from the distance vector between chunk 0 and " << size[2] - 1
						<< " " << distVecNorm;
			} else {
				LOG( Debug, info )
						<< "used the distance between chunk 0 and " << size[2] - 1
						<< " to synthesize the missing sliceVec as " << distVecNorm;
				propertyValue( "sliceVec" ) = distVecNorm;
			}
		}

		const util::fvector4 &voxeSize = getProperty<util::fvector4>( "voxelSize" );

		const Chunk &next = chunkAt( 1 );

		if ( next.hasProperty( "indexOrigin" ) ) {
			const util::fvector4 nextV = next.getProperty<util::fvector4>( "indexOrigin" );
			const float sliceDist = ( nextV - thisV ).len() - voxeSize[2];

			if ( sliceDist > 0 ) {
				const float inf = std::numeric_limits<float>::infinity();

				if ( ! hasProperty( "voxelGap" ) ) { // @todo check this
					setProperty( "voxelGap", util::fvector4( 0, 0, inf, 0 ) );
				}

				util::fvector4 &voxelGap = propertyValue( "voxelGap" )->castTo<util::fvector4>(); //if there is no voxelGap yet, we create it

				if ( voxelGap[2] != inf ) {
					if ( ! util::fuzzyEqual( voxelGap[2], sliceDist, 5e1 ) ) {
						LOG_IF( ! util::fuzzyEqual( voxelGap[2], sliceDist, 5e1 ), Runtime, warning )
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

	//if we have read- and phase- vector
	if ( hasProperty( "readVec" ) && hasProperty( "phaseVec" ) ) {
		util::fvector4 &read = propertyValue( "readVec" )->castTo<util::fvector4>();
		util::fvector4 &phase = propertyValue( "phaseVec" )->castTo<util::fvector4>();
		LOG_IF( read.dot( phase ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
		const util::fvector4 crossVec = util::fvector4( //we could use their cross-product as sliceVector
											read[1] * phase[2] - read[2] * phase[1],
											read[2] * phase[0] - read[0] * phase[2],
											read[0] * phase[1] - read[1] * phase[0]
										);

		if ( hasProperty( "sliceVec" ) ) {
			util::fvector4 &sliceVec = propertyValue( "sliceVec" )->castTo<util::fvector4>(); //get the slice vector
			LOG_IF( ! crossVec.fuzzyEqual( sliceVec, 1e3 ), Runtime, warning )
					<< "The existing sliceVec " << sliceVec
					<< " differs from the cross product of the read- and phase vector " << crossVec;
		} else {
			// We dont know anything about the slice-direction
			// we just guess its along the positive cross-product between read- and phase direction
			// so at least warn the user if we do that long shot
			LOG( Runtime, warning )
					<< "used the cross product between readVec and phaseVec as sliceVec:"
					<< crossVec << ". That might be wrong!";
			setProperty( "sliceVec", crossVec );
		}
	}

	if ( hasProperty( "fov" ) ) {
		util::fvector4 &propFoV = propertyValue( "fov" )->castTo<util::fvector4>();
		util::fvector4 voxelGap;

		if ( hasProperty( "voxelGap" ) ) {
			voxelGap = getProperty<util::fvector4>( "voxelGap" );

			for ( size_t i = 0; i < dims; i++ )
				if ( voxelGap[i] == -std::numeric_limits<float>::infinity() ) {
					LOG( Runtime, info ) << "Ignoring unknown voxel gap in direction " << i;
					voxelGap[i] = 0;
				}
		}

		const util::fvector4 &calcFoV = getFoV( getProperty<util::fvector4>( "voxelSize" ), voxelGap );

		bool ok = true;

		for ( size_t i = 0; i < dims; i++ ) {
			if ( propFoV[i] != -std::numeric_limits<float>::infinity() ) {
				ok &= util::fuzzyEqual( propFoV[i], calcFoV[i] );
			} else
				propFoV[i] = calcFoV[i];
		}

		LOG_IF( ! ok, Runtime, warning )
				<< "The calculated field of view differs from the stored " << propFoV << "/" << calcFoV;
	}

	LOG_IF( ! valid(), Runtime, warning ) << "The image is not valid after reindexing. Missing properties: " << getMissing();
	return clean = valid();
}
bool Image::empty()const
{
	return set.empty();
}

const boost::shared_ptr< Chunk >& Image::chunkPtrAt( size_t at )const
{
	LOG_IF( lookup.empty(), Debug, error ) << "The lookup table is empty. Run reIndex first.";
	LOG_IF( at >= lookup.size(), Debug, error ) << "Index is out of the range of the lookup table (" << at << ">=" << lookup.size() << ").";
	const boost::shared_ptr<Chunk> &ptr = lookup[at];
	LOG_IF( !ptr, Debug, error ) << "There is no chunk at " << at << ". This usually happens in incomplete images.";
	return ptr;
}

Chunk Image::getChunkAt( size_t at, bool copy_metadata )const
{
	Chunk ret( *chunkPtrAt( at ) );

	if( copy_metadata )ret.join( *this ); // copy all metadata from the image in here

	return ret;
}
Chunk &Image::chunkAt( size_t at )
{
	return *chunkPtrAt( at );
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
std::vector< boost::shared_ptr< Chunk > > Image::getChunkList()
{
	checkMakeClean();//lookup is filled by reIndex
	return lookup;
}

std::vector< boost::shared_ptr<const Chunk > > Image::getChunkList()const
{
	LOG_IF( !clean, Debug, error ) << "You shouldn't do this on a non clean image. Run reIndex first.";
	return std::vector< boost::shared_ptr<const Chunk > >( lookup.begin(), lookup.end() );
}


size_t Image::getChunkStride ( size_t base_stride )
{
	LOG_IF( set.empty(), Runtime, error ) << "Trying to get chunk stride in an empty image";
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
		const util::fvector4 firstV = chunkAt( 0 ).getProperty<util::fvector4>( "indexOrigin" );
		const util::fvector4 nextV = chunkAt( base_stride ).getProperty<util::fvector4>( "indexOrigin" );
		const util::fvector4 dist1 = nextV - firstV;

		if( dist1.sqlen() == 0 ) { //if there is no geometric structure anymore - so asume its flat from here on
			LOG( Debug, info ) << "Distance between 0 and " << util::MSubject( base_stride )
							   << " is zero. Assuming there are no dimensional breaks anymore. Returning " << util::MSubject( base_stride );
			return base_stride;
		} else for ( size_t i = base_stride; i < lookup.size() - base_stride; i += base_stride ) {  // compare every follwing distance to that
				const util::fvector4 thisV = chunkAt( i ).getProperty<util::fvector4>( "indexOrigin" );
				const util::fvector4 nextV = chunkAt( i + base_stride ).getProperty<util::fvector4>( "indexOrigin" );
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
	return lookup.size()/set.getHorizontalSize();
}

std::list<util::PropertyValue> Image::getChunksProperties( const util::PropMap::key_type &key, bool unique )const
{
	std::list<util::PropertyValue > ret;

	if( clean ) {
		BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
			const util::PropertyValue &prop = ref->propertyValue( key );

			if ( unique && prop.empty() ) //if unique is requested and the property is empty
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

size_t Image::bytes_per_voxel() const
{
	size_t size = chunkPtrAt( 0 )->bytes_per_voxel();
	BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
		LOG_IF( size != ref->bytes_per_voxel(), Debug, warning )
				<< "Not all voxels have the same byte size (" << size << "!=" << ref->bytes_per_voxel() << "). Using the biggest.";

		if( size < ref->bytes_per_voxel() ) {
			size = ref->bytes_per_voxel();
		}
	}
	return size;
}

ImageList::ImageList() {}
ImageList::ImageList( ChunkList src )
{
	for ( ChunkList::iterator i = src.begin(); i != src.end(); ) {
		if ( ! ( *i )->valid() ) { // drop invalid chunks
			LOG( Runtime, error )
					<< "Ignoring invalid chunk. Missing properties: " << ( *i )->getMissing();
			src.erase( i++ );
		} else
			i++;
	}

	size_t errcnt = 0;

	while ( !src.empty() ) {
		LOG( Debug, info ) << src.size() << " Chunks left to be distributed.";
		value_type buff( new Image );
		size_t cnt = 0;

		for ( ChunkList::iterator i = src.begin(); i != src.end(); ) { // for all remaining chunks
			if ( buff->insertChunk( **i ) ) {
				src.erase( i++ );
				cnt++;
			} else
				i++;
		}

		if ( !buff->empty() ) {
			LOG( Debug, info ) << "Reindexing image with " << cnt << " chunks.";

			if ( buff->reIndex() ) {
				if ( buff->valid() ) {
					push_back( buff );
					LOG( Runtime, info ) << "Image " << size() << " with size " << buff->getSizeAsString() <<  " done.";
				} else {
					LOG( Runtime, error )
							<< "Cannot insert image. Missing properties: " << buff->getMissing();
					errcnt += cnt;
				}
			} else {
				LOG( Runtime, info ) << "Skipping broken image.";
				errcnt += cnt;
			}
		}
	}

	LOG_IF( errcnt, Runtime, warning ) << "Dropped " << errcnt << " chunks because they didn't form valid images";
}

void Image::getMinMax ( util::TypeReference &min, util::TypeReference &max ) const
{
	LOG_IF( ! min.empty(), Debug, warning ) << "Running getMinMax using non empty min. It will be reset.";
	LOG_IF( ! max.empty(), Debug, warning ) << "Running getMinMax using non empty max. It will be reset.";
	min = util::TypeReference();
	max = util::TypeReference();
	BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
		ref->getMinMax( min, max );
	}
}

std::pair< util::TypeReference, util::TypeReference > Image::getScalingTo(short unsigned int targetID, autoscaleOption scaleopt) const
{
	LOG_IF(!clean,Runtime,error) << "You should run reIndex before running this";
	util::TypeReference min,max;
	getMinMax(min,max);
	bool unique=true;
	const std::vector<boost::shared_ptr<const Chunk> > chunks=getChunkList();
	BOOST_FOREACH(const boost::shared_ptr<const Chunk> &ref, chunks){ //find a chunk which would be converted
		if(targetID != ref->typeID()){
			LOG_IF(ref->getScalingTo(targetID,*min,*max,scaleopt).first.empty() || ref->getScalingTo(targetID,*min,*max,scaleopt).second.empty(),Debug,error)
				<< "Returning an invalid scaling. This is bad!";
			return ref->getScalingTo(targetID,*min,*max,scaleopt); // and ask that for the scaling
		}
	}
	return std::make_pair( //ok seems like no conversion is needed - return 1/0
			util::TypeReference(util::Type<uint8_t>(1)),
			util::TypeReference(util::Type<uint8_t>(0))
	);
}

size_t Image::cmp( const isis::data::Image &comp ) const
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

	for ( size_t i = 0; i < volume(); i += increment ) {
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
		ret += c1.cmpRange( c1pair1.second, c1pair2.second, c2, c2pair1.second );
	}

	return ret;
}

Image::orientation Image::getMainOrientation()const
{
	LOG_IF( ! valid() || ! clean, Debug, warning ) << "You should not run this on non clean image. Run reIndex first.";
	util::fvector4 read = getProperty<util::fvector4>( "readVec" );
	util::fvector4 phase = getProperty<util::fvector4>( "phaseVec" );
	read.norm();
	phase.norm();
	LOG_IF( read.dot( phase ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
	const util::fvector4 crossVec = util::fvector4(
										read[1] * phase[2] - read[2] * phase[1],
										read[2] * phase[0] - read[0] * phase[2],
										read[0] * phase[1] - read[1] * phase[0]
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

unsigned short Image::typeID() const
{
	unsigned int mytypeID = chunkPtrAt( 0 )->typeID();
	size_t tmpBytesPerVoxel = 0;
	util::TypeReference min,max;
	getMinMax(min,max);
	LOG(Debug,info) << "Determining  datatype of image with the value range " << min << " to " << max;
	if(min->typeID() == max->typeID()){ // ok min and max are the same type - trivial case
		return min->typeID()  << 8; // btw: we do the shift, because min and max are Type - but we want the ID's TypePtr
	} else if(min->fitsInto(max->typeID())){ // if min fits into the type of max, use that
		return max->typeID()  << 8; //@todo maybe use a global static function here instead of a obscure shit operation
	} else if(max->fitsInto(min->typeID())){ // if max fits into the type of min, use that
		return min->typeID()  << 8;
	} else {
		LOG(Runtime,error) << "Sorry I dont know which datatype I should use. (" << min->typeName() << " or " << max->typeName() <<")";
		throw(std::logic_error("type selection failed"));
		return std::numeric_limits<unsigned char>::max();
	}
}
std::string Image::typeName() const
{
	return util::getTypeMap()[typeID()];
}

bool Image::makeOfTypeID( short unsigned int ID )
{
	// get value range of the image for the conversion
	scaling_pair scale=getScalingTo(ID);

	LOG( Debug, info ) << "Computed scaling of the original image data: [" << scale << "]";
	bool retVal = true;
	//we want all chunks to be of type ID - so tell them
	BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, lookup ) {
		retVal &= ref->makeOfTypeID( ID, scale );
	}
	return retVal;
}

size_t Image::spliceDownTo( dimensions dim ) //readDim = 0, phaseDim, sliceDim, timeDim
{
	if( lookup[0]->relevantDims() < ( size_t ) dim ) {
		LOG( Debug, error ) << "The dimensionality of the chunks of this image is already below " << dim << " cannot splice it.";
		return 0;
	} else if( lookup[0]->relevantDims() == ( size_t ) dim ) {
		LOG( Debug, info ) << "Skipping useless splicing, relevantDims is allready " << lookup[0]->relevantDims();
		return lookup.size();
	}

	LOG_IF( lookup[0]->relevantDims() == ( size_t ) dim, Debug, info ) << "Running useless splice, the dimensionality of the chunks of this image is already " << dim;
	LOG_IF( hasProperty( "acquisitionTime" ) || lookup[0]->hasProperty( "acquisitionTime" ), Debug, warning ) << "Splicing images with acquisitionTime will cause you lots of trouble. You should remove that before.";
	util::FixedVector<size_t, 4> size = getSizeAsVector();

	for( int i = 0; i < dim; i++ )
		size[i] = 1;

	// get a list of needed properties (everything which is missing in a newly created chunk plus everything which is needed for autosplice)
	const std::list<std::string> splice_needed = util::string2list<std::string>( "voxelSize,voxelGap,readVec,phaseVec,sliceVec,indexOrigin,acquisitionNumber", ',' );
	util::PropMap::key_list needed = MemChunk<short>( 1 ).getMissing();
	needed.insert( splice_needed.begin(), splice_needed.end() );
	struct splicer {
		dimensions m_dim;
		Image &m_image;
		size_t m_amount;
		splicer( dimensions dim, size_t amount, Image &image ): m_dim( dim ), m_image( image ), m_amount( amount ) {}
		void operator()( const Chunk &ch ) {
			const size_t topDim = ch.relevantDims() - 1;

			if( topDim >= ( size_t ) m_dim ) { // ok we still have to splice that
				const size_t subSize = m_image.getSizeAsVector()[topDim];
				assert( !( m_amount % subSize ) ); // there must not be any "remaining"
				splicer sub( m_dim, m_amount / subSize, m_image );
				BOOST_FOREACH( ChunkList::const_reference ref, ch.autoSplice( m_amount / subSize ) ) {
					sub( *ref );
				}
			} else { // seems like we're done - insert it into the image
				assert( ch.relevantDims() == ( size_t ) m_dim ); // index of the higest dim>1 (ch.relevantDims()-1) shall be equal to the dim below the requested splicing (m_dim-1)
				m_image.insertChunk( ch );
			}
		}
	};
	std::vector<boost::shared_ptr<Chunk> > buffer = lookup; // store the old lookup table
	lookup.clear();
	set.clear(); // clear the image, so we can insert the splices
	//static_cast<util::PropMap::base_type*>(this)->clear(); we can keep the common properties - they will be merged with thier own copies from the chunks on the next reIndex
	splicer splice( dim, size.product(), *this );
	BOOST_FOREACH( boost::shared_ptr<Chunk> &ref, buffer ) {
		BOOST_FOREACH( const std::string & need, needed ) { //get back properties needed for the
			if( !ref->hasProperty( need ) && this->hasProperty( need ) ) {
				ref->propertyValue( need ) = this->propertyValue( need );
			}
		}
		splice( *ref );
	}
	reIndex();
	return lookup.size();
}


} // END namespace data
} // END namespace isis
