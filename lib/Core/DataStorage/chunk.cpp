//
// C++ Implementation: chunk
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "DataStorage/chunk.hpp"
#include <boost/foreach.hpp>
#include <limits>



namespace isis
{
namespace data
{
namespace _internal
{

ChunkBase::ChunkBase ( size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim )
{
	const size_t idx[] = {firstDim, secondDim, thirdDim, fourthDim};
	init( idx );
	addNeededFromString( needed );
	LOG_IF( NDimensional<4>::volume() == 0, Debug, warning )
			<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid";
}

ChunkBase::~ChunkBase() { }

}

Chunk::Chunk( const TypePtrReference &src, size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim ):
	_internal::ChunkBase( firstDim, secondDim, thirdDim, fourthDim ),
	TypePtrReference( src )
{
	assert( ( *this )->len() == volume() );
}

Chunk Chunk::cloneToMem( size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim )const
{
	util::FixedVector<size_t, 4> newSize = sizeToVector();

	if ( firstDim )newSize[0] = firstDim;

	if ( secondDim )newSize[1] = secondDim;

	if ( thirdDim )newSize[2] = thirdDim;

	if ( fourthDim )newSize[3] = fourthDim;

	const TypePtrReference cloned( get()->cloneToMem( newSize.product() ) );
	return Chunk( cloned, newSize[0], newSize[1], newSize[2], newSize[3] );
}

bool Chunk::makeOfTypeId( short unsigned int id )
{
	if( typeID() != id ) {
		std::pair<util::TypeReference,util::TypeReference> scale=getScalingTo(id);
		return makeOfTypeId( id, *scale.first, *scale.second );
	}
	return true;
}

bool Chunk::makeOfTypeId( short unsigned int id, const util::_internal::TypeBase &scale, const util::_internal::TypeBase &offset )
{
	if( typeID() != id ) { // if its not the same type - replace the internal TypePtr by a new returned from TypePtrBase::copyToNewById
		TypePtrReference newPtr = getTypePtrBase().copyToNewById( id, scale, offset ); // create a new TypePtr of type id and store it in a TypePtrReference

		if( newPtr.empty() ) // if the reference is empty the conversion failed
			return false;

		static_cast<TypePtrReference &>( *this ) = newPtr; // otherwise replace my own TypePtr with the new one
	}

	return true;
}

size_t Chunk::bytes_per_voxel()const
{
	return get()->bytes_per_elem();
}
std::string Chunk::typeName()const
{
	return get()->typeName();
}
unsigned short Chunk::typeID()const
{
	return get()->typeID();
}

void Chunk::copyLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {sizeToVector()[0] - 1, secondDimS, thirdDimS, fourthDimS};
	const size_t idx3[] = {0, secondDimD, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}

void Chunk::copySlice( size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {sizeToVector()[0] - 1, sizeToVector()[1] - 1, thirdDimS, fourthDimS};
	const size_t idx3[] = {0, 0, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}

void Chunk::copyRange( const size_t source_start[], const size_t source_end[], Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! rangeCheck( source_start ), Debug, error )
			<< "Copy start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << sizeToString() << ") at the source chunk";
	LOG_IF( ! rangeCheck( source_end ), Debug, error )
			<< "Copy end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << sizeToString() << ") at the source chunk";
	LOG_IF( ! dst.rangeCheck( destination ), Debug, error )
			<< "Index " << util::FixedVector<size_t, 4>( destination )
			<< " is out of range (" << sizeToString() << ") at the destination chunk";
	const size_t sstart = dim2Index( source_start );
	const size_t send = dim2Index( source_end );
	const size_t dstart = dst.dim2Index( destination );
	get()->copyRange( sstart, send, *dst, dstart );
}

size_t Chunk::cmpRange( size_t start, size_t end, const Chunk &dst, size_t destination ) const
{
	return get()->cmp( start, end, *dst, destination );
}
size_t Chunk::cmpRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! rangeCheck( source_start ), Debug, error )
			<< "memcmp start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << sizeToString() << ") at the first chunk";
	LOG_IF( ! rangeCheck( source_end ), Debug, error )
			<< "memcmp end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << sizeToString() << ") at the first chunk";
	LOG_IF( ! dst.rangeCheck( destination ), Debug, error )
			<< "Index " << util::FixedVector<size_t, 4>( destination )
			<< " is out of range (" << sizeToString() << ") at the second chunk";
	LOG( Debug, verbose_info )
			<< "Comparing range from " << util::FixedVector<size_t, 4>( source_start ) << " to " << util::FixedVector<size_t, 4>( source_end )
			<< " and " << util::FixedVector<size_t, 4>( destination );
	const size_t sstart = dim2Index( source_start );
	const size_t send = dim2Index( source_end );
	const size_t dstart = dst.dim2Index( destination );
	return cmpRange( sstart, send, dst, dstart );
}
size_t Chunk::cmpLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, secondDimD, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, secondDimD, thirdDimD, fourthDimD};
	return cmpRange( idx1, idx2, dst, idx3 );
}
size_t Chunk::cmpSlice( size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, 0, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, sizeToVector()[1] - 1, thirdDimD, fourthDimD};
	return cmpRange( idx1, idx2, dst, idx3 );
}
void Chunk::getMinMax ( util::TypeReference &min, util::TypeReference &max ) const
{
	return operator*().getMinMax( min, max );
}

std::pair<util::TypeReference,util::TypeReference> Chunk::getScalingTo( unsigned short typeID, autoscaleOption scaleopt )const{
	util::TypeReference min,max;
	getMinMax(min,max);
	return operator*().getScalingTo(typeID,*min,*max,scaleopt);
}
std::pair<util::TypeReference,util::TypeReference> Chunk::getScalingTo( unsigned short typeID, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt )const{
	return operator*().getScalingTo(typeID,min,max,scaleopt);
}

Chunk &Chunk::operator=( const Chunk &ref )
{
	_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
	TypePtrReference::operator=( static_cast<const TypePtrReference &>( ref ) ); // copy the reference of ref's data
	return *this;
}

ChunkList Chunk::autoSplice ( uint32_t acquisitionNumberStride )const
{
	if ( !valid() ) {
		LOG( Runtime, error ) << "Cannot splice invalid Chunk (missing properties are " << this->getMissing() << ")";
		return ChunkList();
	}

	util::fvector4 offset;
	const util::fvector4 voxelSize = propertyValue( "voxelSize" )->cast_to<util::fvector4>();
	util::fvector4 voxelGap;

	if( hasProperty( "voxelGap" ) )
		voxelGap = propertyValue( "voxelGap" )->cast_to<util::fvector4>();

	const util::fvector4 distance = voxelSize + voxelGap;
	int32_t atDim = relevantDims() - 1;

	switch( atDim ) { // init offset with the given direction
	case readDim :
		offset = this->propertyValue( "readVec" )->cast_to<util::fvector4>();
		break;
	case phaseDim:
		offset = this->propertyValue( "phaseVec" )->cast_to<util::fvector4>();
		break;
	case sliceDim:

		if( this->hasProperty( "sliceVec" ) ) {
			offset = this->propertyValue( "sliceVec" )->cast_to<util::fvector4>();
		} else {
			const util::fvector4 read = this->propertyValue( "readVec" )->cast_to<util::fvector4>();
			const util::fvector4 phase = this->propertyValue( "phaseVec" )->cast_to<util::fvector4>();
			assert( util::fuzzyEqual<float>( read.sqlen(), 1 ) );
			assert( util::fuzzyEqual<float>( phase.sqlen(), 1 ) );
			offset[0] = read[1] * phase[2] - read[2] * phase[1];
			offset[1] = read[2] * phase[0] - read[0] * phase[2];
			offset[2] = read[0] * phase[1] - read[1] * phase[0];
		}

		break;
	case timeDim :
		offset = util::fvector4( 0, 0, 0, 1 );
	}

	// prepare some attributes
	assert( util::fuzzyEqual<float>( offset.sqlen(), 1 ) ); // it should be norm here
	const util::fvector4 indexOriginOffset = offset * distance[atDim];
	size_t cnt = 0;
	LOG( Debug, info ) << "Splicing chunk at dimenstion " << atDim + 1 << " with indexOrigin stride " << indexOriginOffset << " and acquisitionNumberStride " << acquisitionNumberStride;
	ChunkList ret = splice( ( dimensions )atDim ); // do low level splice - get the chunklist
	BOOST_FOREACH( ChunkList::reference ref, ret ) { // adapt some metadata in them
		util::fvector4 &orig = ref->propertyValue( "indexOrigin" )->cast_to<util::fvector4>();
		uint32_t &acq = ref->propertyValue( "acquisitionNumber" )->cast_to<u_int32_t>();
		orig = orig + indexOriginOffset * cnt;
		acq += acquisitionNumberStride * cnt; //@todo this might cause trouble if we try to insert this chunks into an image
		cnt++;
	}
	return ret;
}

ChunkList Chunk::splice ( dimensions atDim )const
{
	ChunkList ret;
	//@todo should be locking
	typedef std::vector<TypePtrReference> TypePtrList;
	const util::FixedVector<size_t, n_dims> wholesize = sizeToVector();
	util::FixedVector<size_t, n_dims> spliceSize;
	spliceSize.fill( 1 ); //init size of one chunk-splice to 1x1x1x1
	//copy the relevant dimensional sizes from wholesize (in case of sliceDim we copy only the first two elements of wholesize - making slices)
	spliceSize.copyFrom( &wholesize[0], &wholesize[atDim] );
	//get the spliced TypePtr's (the volume of the requested dims is the split-size - in case of sliceDim it is rows*columns)
	const TypePtrList pointers = this->getTypePtrBase().splice( spliceSize.product() );
	//create new Chunks from this TypePtr's
	BOOST_FOREACH( TypePtrList::const_reference ref, pointers ) {
		boost::shared_ptr<Chunk> spliced( new Chunk( ref, spliceSize[0], spliceSize[1], spliceSize[2], spliceSize[3] ) );
		static_cast<util::PropMap &>( *spliced ) = static_cast<const util::PropMap &>( *this ); //copy the metadate of ref
		ret.push_back( spliced ); // store splice for return
	}
	return ret;
}
bool Chunk::swapAlong( Chunk &dst, const size_t dim, bool convertTransform ) const
{
	size_t dims[] = { dimSize( 0 ), dimSize( 1 ), dimSize( 2 ), dimSize( 3 ) };
	dst.join( static_cast<util::PropMap>( *this ), false );

	if ( get()->swapAlong( *dst, dim, dims ) ) {
		if ( convertTransform ) {
			util::fvector4 read = getProperty<util::fvector4>( "readVec" );
			util::fvector4 phase = getProperty<util::fvector4>( "phaseVec" );
			util::fvector4 slice = getProperty<util::fvector4>( "sliceVec" );
			util::fvector4 origin = getProperty<util::fvector4>( "indexOrigin" );
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
			dst.transformCoords( T );
		}

		return true;
	} else return false;
}

}
}

