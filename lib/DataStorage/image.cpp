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

#include "image.hpp"
#include "CoreUtils/vector.hpp"
#include <boost/foreach.hpp>
#include "CoreUtils/property.hpp"

#define _USE_MATH_DEFINES 1
#include <math.h>
#include <cmath>

namespace isis
{
namespace data
{

namespace _internal
{

bool image_chunk_order::operator() ( const data::Chunk &a, const data::Chunk &b )const
{
	//@todo exception ??
	LOG_IF( ! a.hasProperty( "indexOrigin" ), Debug, error )
			<< "The chunk has no position, it can not be sorted into the image.";
	LOG_IF( ! a.hasProperty( "acquisitionNumber" ), Debug, warning )
			<< "The chunk has no acquisitionNumber, it may not be sorted into the image.";
	const util::fvector4 &posA = a.getProperty<util::fvector4>( "indexOrigin" );
	const util::fvector4 &posB = b.getProperty<util::fvector4>( "indexOrigin" );

	if ( posA.lexical_less_reverse( posB ) ) { //if chunk is "under" the other - put it there
		LOG( Debug, verbose_info )
				<< "Successfully sorted chunks by position"
				<< " (" << posA << " below " << posB << ")";
		return true;
	}

	if ( posA == posB ) { //if the chunks have the same position, check if they can be sorted by time
		if ( a.hasProperty( "acquisitionTime" ) && b.hasProperty( "acquisitionTime" ) ) {
			const float aTime = a.getProperty<float>( "acquisitionTime" );
			const float bTime = b.getProperty<float>( "acquisitionTime" );

			if ( aTime < bTime ) {
				LOG( Debug, info )
						<< "Fallback sorted chunks by time"
						<< " (" << aTime << " before " << bTime << ")";
				return true;
			} else if ( bTime < aTime ) {
				return false;
			}
		}

		//if acquisitionTime is equal as well (or missing) fall back to acquisitionNumber
		const uint32_t aNumber = a.getProperty<uint32_t>( "acquisitionNumber" );
		const uint32_t bNumber = b.getProperty<uint32_t>( "acquisitionNumber" );

		if ( aNumber < bNumber ) {
			//if they at least have different acquisitionNumber
			LOG( Debug, info )
					<< "Fallback sorted chunks by acquisition order"
					<< " (" << aNumber << " before " << bNumber << ")";
			return true;
		}

		LOG_IF( aNumber == bNumber, Runtime, info )
				<< "The Chunks from \"" << a.propertyValue( "source" ).toString(false) << "\" and \""
				<< b.propertyValue( "source" ).toString(false) << "\" seem to be equal, won't insert";
	}

	return false;
}

}

Image::Image ( _internal::image_chunk_order lt ) : set ( lt ), clean( false )
{
	addNeededFromString( needed );
}


bool Image::insertChunk ( const Chunk &chunk )
{
	if ( ! chunk.valid() ) {
		LOG( Runtime, error )
				<< "Cannot insert chunk. Missing properties: " << chunk.getMissing();
		return false;
	}

	LOG_IF(chunk.getProperty<util::fvector4>("indexOrigin")[3]!=0,Debug,warning)
		<< " inserting chunk with nonzero at the 4th position - you shouldn't use the fourth dim for the time (use acquisitionTime)";

	if ( ! set.empty() ) {
		const Chunk &first = *set.begin();

		//if our first chunk and the incoming chunk do have different size, skip it
		if ( first.sizeToVector() != chunk.sizeToVector() ) {
			LOG( Debug, info )
					<< "Ignoring chunk with different size. (" << chunk.sizeToString() << "!=" << set.begin()->sizeToString() << ")";
			return false;
		}


		if ( first.hasProperty("readVec") && chunk.hasProperty("readVec") && first.propertyValue( "readVec" ) != chunk.propertyValue( "readVec" ) ) {
			LOG( Debug, info )
					<< "Ignoring chunk with different readVec. (" << chunk.propertyValue( "readVec" ) << "!=" << first.propertyValue( "readVec" ) << ")";
			return false;
		}

		if ( first.hasProperty("phaseVec") && chunk.hasProperty("phaseVec") && first.propertyValue( "phaseVec" ) != chunk.propertyValue( "phaseVec" ) ) {
			LOG( Debug, info )
					<< "Ignoring chunk with different phaseVec. (" << chunk.propertyValue( "phaseVec" ) << "!=" << first.propertyValue( "phaseVec" ) << ")";
			return false;
		}

		//if our first chunk and the incoming chunk do have sequenceNumber and it differs, skip it
		if ( first.hasProperty("coilChannelMask") && chunk.hasProperty("coilChannelMask") && first.propertyValue( "coilChannelMask" ) != chunk.propertyValue( "coilChannelMask" ) ) {
			LOG( Debug, info )
					<< "Ignoring chunk because its coilChannelMask doesn't fit ("
					<< first.propertyValue( "coilChannelMask" ) << "!=" << chunk.propertyValue( "coilChannelMask" )
					<< ")";
			return false;
		}
		//if our first chunk and the incoming chunk do have sequenceNumber and it differs, skip it
		if ( first.hasProperty("sequenceNumber") && chunk.hasProperty("sequenceNumber") && first.propertyValue( "sequenceNumber" ) != chunk.propertyValue( "sequenceNumber" ) ) {
			LOG( Debug, info )
					<< "Ignoring chunk because its sequenceNumber doesn't fit ("
					<< first.propertyValue( "sequenceNumber" ) << "!=" << chunk.propertyValue( "sequenceNumber" )
					<< ")";
			return false;
		}
	} else {
		LOG( Debug, verbose_info ) << "Inserting 1st chunk";
	}

	if ( set.insert( chunk ).second ) {
		clean = false;
		return true;
	} else return false;
}


bool Image::reIndex()
{
	if ( set.empty() ) {
		clean = true;
		LOG( Debug, warning ) << "Reindexing an empty image is useless.";
		return true;
	}

	//redo lookup table
	size_t timesteps = 1;
	const size_t chunks = set.size();
	lookup.resize( chunks );

	util::FixedVector<size_t, n_dims> size; //storage for the size of the chunk structure
	size.fill(1);

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//// Non Geometric sorting (put chunks with the same geometric position at the highest dimension)
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//check for chunks with same position
	if ( chunks > 1 ) {
		ChunkSet::iterator it = set.begin();

		for ( size_t i = 0; i < ( chunks - 1 ); timesteps++, i++ ) {
			const util::fvector4 &here = it->getProperty<util::fvector4>( "indexOrigin" );
			const util::fvector4 &next = ( ++it )->getProperty<util::fvector4>( "indexOrigin" );

			if ( here != next )break;
		}

		LOG_IF( chunks % timesteps, Debug, error )
				<< "The number timesteps does not fit the number of chunks. Reindexing will fail.";
		LOG( Debug, info ) << "Found " << timesteps << " chunks per position assuming them as timesteps";
		size[n_dims-1]=timesteps; //store non geometric amount at the end (timeDim)
	}

	if ( chunks > timesteps && chunks % timesteps == 0 ) {
		const size_t chunksets = chunks / timesteps;
		//sort in the chunks (assume the chunkset to be an Matrix where m represents the timesteps, then transpose it )
		size_t idx = 0;

		for ( ChunkSet::iterator it = set.begin(); it != set.end(); it++, idx++ ) {
			const size_t i = idx % timesteps;
			const size_t j = idx / timesteps;
			lookup[i *chunksets+j] = it;
			LOG( Debug, verbose_info ) << "Putting " << idx << " at " << i *chunksets + j;;
		}
	} else {
		size_t idx = 0;
		ChunkSet::iterator it = set.begin();

		while ( it != set.end() )
			lookup[idx++] = it++;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//// geometric sorting (put chunks with distinct geometric position at the remaining dimensions)
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//get primary attributes from first chunk
	const Chunk &first = *set.begin();
	const unsigned short chunk_dims = first.relevantDims();
	chunkVolume = first.volume();
	//copy sizes of the chunks to to the first chunk_dims sizes of the image

	//get indexOrigin from the geometrically first chunk
	propertyValue( "indexOrigin" ) = first.propertyValue( "indexOrigin" );

	//if there are many chunks, they must leave at least on dimension to the image to sort them in
	if ( chunk_dims >= Image::n_dims ) {
		if ( lookup.size() > 1 ) {
			LOG( Runtime, error )
					<< "Cannot handle multiple Chunks, if they have more than "
					<< Image::n_dims - 1 << " dimensions";
			return false;
		}

		//if there is only one chunk, its ok - the image will consist only of this one,
		//and commonGet will allways return <0,set.begin()->dim2Index()>
		//thus in this case voxel() equals set.begin()->voxel()
	} else {// OK there is at least one dimension to sort in the chunks
		// check the chunks for at least one dimensional break - use that for the size of that dimension
		const size_t dummy_var_for_retarded_compilers=getChunkStride();
		size[chunk_dims] =  dummy_var_for_retarded_compilers ? dummy_var_for_retarded_compilers : 1;
		const unsigned short sortDims = n_dims - (size[n_dims-1]>1?1:0); // dont use the uppermost dim, if the timesteps are allready there

		for ( unsigned short i = chunk_dims + 1; i < sortDims; i++ ){ //if there are dimensions left figure out their size
			const size_t dummy_var_for_retarded_compilers=getChunkStride( size.product() ) / size.product();
			if(dummy_var_for_retarded_compilers)
				size[i] =  dummy_var_for_retarded_compilers;
		}
	}

	//Clean up the properties
	//@todo might fail if the image contains a prop that differs to that in the Chunks (which is equal in the chunks)
	util::PropMap common;
	std::set<std::string> uniques;
	set.begin()->toCommonUnique( common, uniques, true );

	for ( ChunkIterator i = ++chunksBegin(); i != chunksEnd(); i++ ) {
		i->toCommonUnique( common, uniques, false );
	}

	LOG( Debug, info ) << uniques.size() << " Chunk-unique properties found in the Image";
	LOG_IF( uniques.size(), Debug, verbose_info ) << util::list2string( uniques.begin(), uniques.end(), ", " );
	join( common );
	LOG_IF( ! common.empty(), Debug, verbose_info ) << "common properties saved into the image " << common;

	//remove common props from the chunks
	for ( size_t i = 0; i != lookup.size(); i++ )
		getChunkAt( i ).remove( common, true );

	LOG_IF( ! common.empty(), Debug, verbose_info ) << "common properties removed from " << set.size() << " chunks: " << common;

	// add the chunk-size to the image-size
	for ( unsigned short i = 0; i < chunk_dims; i++ )
		size[i] = first.dimSize( i );

	init( size );
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//reconstruct some redundant information, if its missing
	//////////////////////////////////////////////////////////////////////////////////////////////////
	const std::string vectors[]={"readVec","phaseVec","sliceVec"};
	BOOST_FOREACH(const std::string &ref,vectors){
		if ( hasProperty( ref ) ){
			util::PropertyValue &prop=propertyValue( ref );
			LOG_IF(!prop->is<util::fvector4>(),Debug,error) << "Using " << prop->typeName() << " as " << util::Type<util::fvector4>::staticName();
			util::fvector4 &vec = prop->cast_to_Type<util::fvector4>();
			LOG_IF( vec.len() == 0, Runtime, error )
					<< "The existing " << ref << " " << vec << " has the length zero. Thats bad, because I'm going to normalize it.";
			vec.norm();
		}
	}

	//if we have at least two slides (and have slides (with different positions) at all)
	if ( chunk_dims == 2 && size[2] > 1 && lookup[0]->hasProperty( "indexOrigin" ) ) {
		const util::fvector4 thisV = lookup[0]->getProperty<util::fvector4>( "indexOrigin" );

		if ( lookup[size[2] - 1]->hasProperty( "indexOrigin" ) ) {
			const util::fvector4 lastV = lookup[size[2] - 1]->getProperty<util::fvector4>( "indexOrigin" );
			//check the slice vector
			util::fvector4 distVecNorm = lastV - thisV;
			LOG_IF( distVecNorm.len() == 0, Runtime, error )
					<< "The distance between the the first and the last chunk is zero. Thats bad, because I'm going to normalize it.";
			distVecNorm.norm();

			if ( hasProperty( "sliceVec" ) ) {
				const util::fvector4 sliceVec = getProperty<util::fvector4>("sliceVec" );
				LOG_IF(! distVecNorm.fuzzyEqual(sliceVec), Runtime, warning )
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

		if ( lookup[1]->hasProperty( "indexOrigin" ) ) {
			const util::fvector4 nextV = lookup[1]->getProperty<util::fvector4>( "indexOrigin" );
			const float sliceDist = ( nextV - thisV ).len() - voxeSize[2];

			if ( sliceDist > 0 ) {
				const float inf = std::numeric_limits<float>::infinity();

				if ( ! hasProperty( "voxelGap" ) ) {
					setProperty( "voxelGap", util::fvector4( inf, inf, inf, inf ) );
				}

				util::fvector4 &voxelGap = propertyValue( "voxelGap" )->cast_to_Type<util::fvector4>(); //if there is no voxelGap yet, we create it

				if ( voxelGap[2] != inf ) {
					if ( ! util::fuzzyEqual( voxelGap[2], sliceDist ) ) {
						LOG_IF( ! util::fuzzyEqual( voxelGap[2], sliceDist ), Runtime, warning )
								<< "The existing slice distance (voxelGap[2]) " << voxelGap[2]
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
		util::fvector4 &read = propertyValue( "readVec" )->cast_to_Type<util::fvector4>();
		util::fvector4 &phase = propertyValue( "phaseVec" )->cast_to_Type<util::fvector4>();

		LOG_IF( read.dot( phase ) > 0.01, Runtime, warning ) << "The cosine between the columns and the rows of the image is bigger than 0.01";
		const util::fvector4 crossVec = util::fvector4( //we could use their cross-product as sliceVector
											read[1] * phase[2] - read[2] * phase[1],
											read[2] * phase[0] - read[0] * phase[2],
											read[0] * phase[1] - read[1] * phase[0]
										);

		if ( hasProperty( "sliceVec" ) ) {
			util::fvector4 &sliceVec = propertyValue( "sliceVec" )->cast_to_Type<util::fvector4>(); //get the slice vector
			LOG_IF( ! crossVec.fuzzyEqual( sliceVec ), Runtime, warning )
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
		util::fvector4 &propFoV = propertyValue( "fov" )->cast_to_Type<util::fvector4>();
		util::fvector4 voxelGap;

		if ( hasProperty( "voxelGap" ) ) {
			voxelGap = getProperty<util::fvector4>( "voxelGap" );

			for ( size_t i = 0; i < n_dims; i++ )
				if ( voxelGap[i] == -std::numeric_limits<float>::infinity() ) {
					LOG( Runtime, info ) << "Ignoring unknown voxel gap in direction " << i;
					voxelGap[i] = 0;
				}
		}

		const util::fvector4 &calcFoV = getFoV( getProperty<util::fvector4>( "voxelSize" ), voxelGap );

		bool ok = true;

		for ( size_t i = 0; i < n_dims; i++ ) {
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

const Chunk &Image::getChunkAt( size_t at )const
{
	return *( lookup[at] );
}
Chunk &Image::getChunkAt( size_t at )
{
	//@todo we must cast away the const here because std::set has no non-const iterators
	Chunk &ret = const_cast<Chunk &>( *( lookup[at] ) );
	return ret;
}

Chunk &Image::getChunk ( size_t first, size_t second, size_t third, size_t fourth )
{
	if ( ! clean ) {
		LOG( Debug, info )
				<< "Image is not clean. Running reIndex ...";
		reIndex();
	}

	const size_t index = commonGet( first, second, third, fourth ).first;

	return getChunkAt( index );
}

const Chunk &Image::getChunk ( size_t first, size_t second, size_t third, size_t fourth ) const
{
	const size_t index = commonGet( first, second, third, fourth ).first;
	return getChunkAt( index );
}

size_t Image::getChunkStride ( size_t base_stride )
{
	if ( set.empty() ) {
		LOG( Runtime, error ) << "Trying to get chunk stride in an empty image";
		return 0;
	} else if ( lookup.empty() ) {
		LOG( Debug, error ) << "Lookup table for chunks is empty. Do reIndex() first!";
		return 0;
	} else if ( lookup.size() >= 4 * base_stride ) {
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
		const util::fvector4 firstV = lookup[0]->getProperty<util::fvector4>( "indexOrigin" );
		const util::fvector4 nextV = lookup[base_stride]->getProperty<util::fvector4>( "indexOrigin" );
		const util::fvector4 dist1 = nextV - firstV;

		if(dist1.sqlen()==0){ //if there is no geometric structure anymore - so asume its flat from here on
				LOG(Debug,info) << "Distance between 0 and " << util::MSubject( base_stride ) 
				<< " is zero. Assuming there are no dimensional breaks anymore. Returning 1";
				return 1;
		} else for ( size_t i = base_stride; i < lookup.size() - base_stride; i += base_stride ) { 	// compare every follwing distance to that
			const util::fvector4 thisV = lookup[i]->getProperty<util::fvector4>( "indexOrigin" );
			const util::fvector4 nextV = lookup[i+base_stride]->getProperty<util::fvector4>( "indexOrigin" );
			const util::fvector4 distFirst = nextV - firstV;
			const util::fvector4 distThis = nextV - thisV;
			LOG( Debug, verbose_info )
					<< "Distance between chunk " << util::MSubject( i ) << " and " << util::MSubject( i + base_stride )
					<< " is " << distThis.len() << ". Distance between 0 and " << util::MSubject( i + base_stride ) <<" is " << distFirst.len();

			if ( distFirst.sqlen() <= distThis.sqlen() ) { // found an dimensional break - leave
				LOG( Debug, info )
						<< "Distance between chunk " << util::MSubject( i + base_stride )
						<< " and 0 is not bigger than the distance between " << util::MSubject( i + base_stride )
						<< " and "<< util::MSubject( i ) << ", assuming dimensional break at " << i + base_stride;
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
		return lookup.size() / base_stride;
	}

	//we didn't find any break, so we assume its a linear image |c c ... c|
	LOG( Debug, info )
			<< "No dimensional break found, assuming it to be at the end (" << lookup.size() << ")";
	return lookup.size();
}

std::list<util::PropertyValue> Image::getChunksProperties( const util::PropMap::key_type &key, bool unique )const
{
	std::list<util::PropertyValue > ret;

	for ( ChunkSet::const_iterator i = set.begin(); i != set.end(); i++ ) {
		const util::PropertyValue &prop = i->propertyValue( key );

		if ( unique && prop.empty() ) //if unique is requested and the property is empty
			continue; //skip it
		else if ( unique && !( ret.empty() ||  prop == ret.back() ) )
			//if unique is requested and the property is equal to the one added before
			continue;//skip it
		else
			ret.push_back( prop );
	}

	return ret;
}

size_t Image::bytes_per_voxel() const
{
	size_t size = set.begin()->bytes_per_voxel();
	BOOST_FOREACH( const Chunk & ref, set ) {
		LOG_IF( size != ref.bytes_per_voxel(), Debug, error )
				<< "Not all voxels have the same byte size. The result might be wrong.";
	}
	return size;
}

Image::ChunkIterator Image::chunksBegin() {return set.begin();}
Image::ChunkIterator Image::chunksEnd() {return set.end();}

isis::data::Image::ConstChunkIterator Image::chunksBegin() const {return set.begin();}
isis::data::Image::ConstChunkIterator Image::chunksEnd() const {return set.end();}

ImageList::ImageList() {}
ImageList::ImageList( ChunkList src )
{
	while ( !src.empty() ) {
		value_type buff( new Image );

		for ( ChunkList::iterator i = src.begin(); i != src.end(); ) {
			if ( ! i->valid() ) {
				const util::PropMap::key_list missing = i->getMissing();
				LOG( Runtime, error )
						<< "Ignoring invalid chunk. Missing properties: " << util::list2string( missing.begin(), missing.end() );
				src.erase( i++ );
			} else {
				if ( buff->insertChunk( *i ) )
					src.erase( i++ );
				else
					i++;
			}
		}

		if ( buff->chunksBegin() != buff->chunksEnd() ) {
			buff->reIndex();

			if ( buff->valid() )
				push_back( buff );
			else {
				const util::PropMap::key_list missing = buff->getMissing();
				LOG( Runtime, error )
						<< "Cannot insert image. Missing properties: " << util::list2string( missing.begin(), missing.end() );
			}
		}
	}
}

void Image::getMinMax ( util::_internal::TypeBase::Reference &min, util::_internal::TypeBase::Reference &max ) const
{
	LOG_IF( ! min.empty(), Debug, warning ) << "Running getMinMax using non empty min. It will be reset.";
	LOG_IF( ! max.empty(), Debug, warning ) << "Running getMinMax using non empty max. It will be reset.";
	min = util::_internal::TypeBase::Reference();
	max = util::_internal::TypeBase::Reference();
	BOOST_FOREACH( const Chunk & ch, set )
	ch.getMinMax( min, max );
}
size_t Image::cmp( const isis::data::Image &comp ) const
{
	size_t ret = 0;
	LOG_IF( ! ( clean && comp.clean ), Debug, error )
			<< "Comparing unindexed images will cause you trouble, run reIndex()!";

	if ( sizeToVector() != comp.sizeToVector() ) {
		LOG( Runtime, warning ) << "Size of images differs (" << sizeToVector() << "/"
								<< comp.sizeToVector() << "). Adding difference to the result.";
		ret += ( sizeToVector() - comp.sizeToVector() ).product();
	}

	util::ivector4 compVect( util::minVector( chunksBegin()->sizeToVector(), comp.chunksBegin()->sizeToVector() ) );
	util::ivector4 start;
	const size_t increment = compVect.product();

	for ( size_t i = 0; i < volume(); i += increment ) {
		const size_t nexti = i + increment - 1;
		const std::pair<size_t, size_t> c1pair1( i / chunkVolume, i % chunkVolume );
		const std::pair<size_t, size_t> c1pair2( nexti / chunkVolume, nexti % chunkVolume );
		const std::pair<size_t, size_t> c2pair1( i / comp.chunkVolume, i % comp.chunkVolume );
		assert( c1pair1.first == c1pair2.first );
		LOG( Debug, verbose_info ) << "Comparing chunks at " << c1pair1.first << " and "   << c2pair1.first;
		const Chunk &c1 = getChunkAt( c1pair1.first );
		const Chunk &c2 = comp.getChunkAt( c2pair1.first );
		LOG( Debug, verbose_info )
				<< "Start positions are " << c1pair1.second << " and " << c2pair1.second
				<< " and the length is " << c1pair2.second - c1pair1.second;
		ret += c1.cmpRange( c1pair1.second, c1pair2.second, c2, c2pair1.second );
	}

	return ret;
}

Image::orientation Image::getMainOrientation()const
{
	LOG_IF(! valid() || ! clean,Debug,warning) << "You should not run this on non clean image. Run reIndex first.";
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
	LOG(Debug,info) << "Angles to vectors are " << a_sagittal << " to x, " << a_coronal << " to y and " << a_axial << " to z";

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

void Image::transformCoords(boost::numeric::ublas::matrix<float> transform)
{
	// create boost::numeric::ublast matrix from orientation vectors
	boost::numeric::ublas::matrix<float> orient(3,3);
	util::fvector4 read = getProperty<util::fvector4>("readVec");
	util::fvector4 phase = getProperty<util::fvector4>("phaseVec");
	util::fvector4 slice = getProperty<util::fvector4>("sliceVec");
	util::fvector4 origin = getProperty<util::fvector4>("indexOrigin");

	// copy orientation vectors into matrix columns
	// readVec
	for(unsigned i = 0;i<3;i++) {
		orient(i,0) = read[i];
	}

	// phaseVec
	for(unsigned i = 0;i<3;i++) {
		orient(i,1) = phase[i];
	}

	// sliceVec
	for(unsigned i = 0;i<3;i++) {
		orient(i,2) = slice[i];
	}

	// copy index origin
	boost::numeric::ublas::vector<float> o(3);
	for (unsigned i = 0;i < 3;i++) {
		o(i) = origin[i];
	}

	// since the orientation matrix is 3x3 orthogonal matrix it holds that
	// orient * orient^T = I, where I is the identity matrix.

	// calculate new orientation matrix --> O_new = O * T
	boost::numeric::ublas::matrix<float> new_orient=
	boost::numeric::ublas::prod(orient,transform);

	// transform index origin into new coordinate space.
	// o_new -> O_new * (O^-1 * o)
	boost::numeric::ublas::vector<float> new_o =
	boost::numeric::ublas::prod(new_orient,
			(boost::numeric::ublas::vector<float>)boost::numeric::ublas::prod(
					(boost::numeric::ublas::matrix<float>)boost::numeric::ublas::trans(orient),o));

	// write origin back to attributes
	for(unsigned i=0;i<3;i++) {
		origin[i] = new_o(i);
	}

	// readVec
	for(unsigned i=0;i<3;i++) {
		read[i] = new_orient(i,0);
	}

	// phaseVec
	for(unsigned i=0;i<3;i++) {
		phase[i] = new_orient(i,1);
	}

	// sliceVec
	for(unsigned i=0;i<3;i++) {
		slice[i] = new_orient(i,2);
	}

	setProperty<util::fvector4>("indexOrigin",origin);
	setProperty<util::fvector4>("readVec",read);
	setProperty<util::fvector4>("phaseVec",phase);
	setProperty<util::fvector4>("sliceVec",slice);

}

} // END namespace data
} // END namespace isis
