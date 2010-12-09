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

#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

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
	addNeededFromString( neededProperties );
	LOG_IF( NDimensional<4>::volume() == 0, Debug, warning )
			<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid";
}

ChunkBase::~ChunkBase() { }

}

Chunk::Chunk( const TypePtrReference &src, size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim ):
	_internal::ChunkBase( firstDim, secondDim, thirdDim, fourthDim ),
	TypePtrReference( src )
{
	assert( ( *this )->length() == volume() );
}

Chunk Chunk::cloneToNew( size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim )const
{
	util::FixedVector<size_t, 4> newSize = getSizeAsVector();

	if ( firstDim )newSize[0] = firstDim;

	if ( secondDim )newSize[1] = secondDim;

	if ( thirdDim )newSize[2] = thirdDim;

	if ( fourthDim )newSize[3] = fourthDim;

	const TypePtrReference cloned( get()->cloneToNew( newSize.product() ) );
	return Chunk( cloned, newSize[0], newSize[1], newSize[2], newSize[3] );
}

bool Chunk::makeOfTypeID( short unsigned int ID )
{
	if( typeID() != ID ) {
		return makeOfTypeID( ID, getScalingTo( ID ) );
	}

	return true;
}

bool Chunk::makeOfTypeID( short unsigned int ID, const scaling_pair &scaling )
{
	if( typeID() != ID ) { // if its not the same type - replace the internal TypePtr by a new returned from TypePtrBase::copyToNewById
		TypePtrReference newPtr = getTypePtrBase().copyToNewByID( ID, scaling ); // create a new TypePtr of type id and store it in a TypePtrReference

		if( newPtr.empty() ) // if the reference is empty the conversion failed
			return false;

		static_cast<TypePtrReference &>( *this ) = newPtr; // otherwise replace my own TypePtr with the new one
	}

	return true;
}

size_t Chunk::bytesPerVoxel()const
{
	return get()->bytesPerElem();
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
	const size_t idx2[] = {getSizeAsVector()[0] - 1, secondDimS, thirdDimS, fourthDimS};
	const size_t idx3[] = {0, secondDimD, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}

void Chunk::copySlice( size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {getSizeAsVector()[0] - 1, getSizeAsVector()[1] - 1, thirdDimS, fourthDimS};
	const size_t idx3[] = {0, 0, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}

void Chunk::copyRange( const size_t source_start[], const size_t source_end[], Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! isInRange( source_start ), Debug, error )
			<< "Copy start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << getSizeAsString() << ") at the source chunk";
	LOG_IF( ! isInRange( source_end ), Debug, error )
			<< "Copy end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << getSizeAsString() << ") at the source chunk";
	LOG_IF( ! dst.isInRange( destination ), Debug, error )
			<< "Index " << util::FixedVector<size_t, 4>( destination )
			<< " is out of range (" << getSizeAsString() << ") at the destination chunk";
	const size_t sstart = getLinearIndex( source_start );
	const size_t send = getLinearIndex( source_end );
	const size_t dstart = dst.getLinearIndex( destination );
	get()->copyRange( sstart, send, *dst, dstart );
}

size_t Chunk::cmpRange( size_t start, size_t end, const Chunk &dst, size_t destination ) const
{
	return get()->compare( start, end, *dst, destination );
}
size_t Chunk::cmpRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! isInRange( source_start ), Debug, error )
			<< "memcmp start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << getSizeAsString() << ") at the first chunk";
	LOG_IF( ! isInRange( source_end ), Debug, error )
			<< "memcmp end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << getSizeAsString() << ") at the first chunk";
	LOG_IF( ! dst.isInRange( destination ), Debug, error )
			<< "Index " << util::FixedVector<size_t, 4>( destination )
			<< " is out of range (" << getSizeAsString() << ") at the second chunk";
	LOG( Debug, verbose_info )
			<< "Comparing range from " << util::FixedVector<size_t, 4>( source_start ) << " to " << util::FixedVector<size_t, 4>( source_end )
			<< " and " << util::FixedVector<size_t, 4>( destination );
	const size_t sstart = getLinearIndex( source_start );
	const size_t send = getLinearIndex( source_end );
	const size_t dstart = dst.getLinearIndex( destination );
	return cmpRange( sstart, send, dst, dstart );
}
size_t Chunk::cmpLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, secondDimD, thirdDimD, fourthDimD};
	const size_t idx3[] = {getSizeAsVector()[0] - 1, secondDimD, thirdDimD, fourthDimD};
	return cmpRange( idx1, idx2, dst, idx3 );
}
size_t Chunk::cmpSlice( size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, 0, thirdDimD, fourthDimD};
	const size_t idx3[] = {getSizeAsVector()[0] - 1, getSizeAsVector()[1] - 1, thirdDimD, fourthDimD};
	return cmpRange( idx1, idx2, dst, idx3 );
}
std::pair<util::TypeReference,util::TypeReference> Chunk::getMinMax ( ) const
{
	return operator*().getMinMax();
}

scaling_pair Chunk::getScalingTo( unsigned short typeID, autoscaleOption scaleopt )const
{
	std::pair<util::TypeReference,util::TypeReference> minmax=getMinMax();
	return operator*().getScalingTo( typeID, *minmax.first, *minmax.second, scaleopt );
}
scaling_pair Chunk::getScalingTo( unsigned short typeID, const std::pair<util::TypeReference,util::TypeReference> &minmax, autoscaleOption scaleopt )const
{
	return operator*().getScalingTo( typeID, minmax, scaleopt );
}
scaling_pair Chunk::getScalingTo( unsigned short typeID, const util::_internal::TypeBase &min, const util::_internal::TypeBase &max, autoscaleOption scaleopt )const
{
	return operator*().getScalingTo( typeID, min, max, scaleopt );
}

Chunk &Chunk::operator=( const Chunk &ref )
{
	_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
	TypePtrReference::operator=( static_cast<const TypePtrReference &>( ref ) ); // copy the reference of ref's data
	return *this;
}

ChunkList Chunk::autoSplice ( uint32_t acquisitionNumberStride )const
{
	if ( !isValid() ) {
		LOG( Runtime, error ) << "Cannot splice invalid Chunk (missing properties are " << this->getMissing() << ")";
		return ChunkList();
	}

	util::fvector4 offset;
	const util::fvector4 voxelSize = propertyValue( "voxelSize" )->castTo<util::fvector4>();
	util::fvector4 voxelGap;

	if( hasProperty( "voxelGap" ) )
		voxelGap = propertyValue( "voxelGap" )->castTo<util::fvector4>();

	const util::fvector4 distance = voxelSize + voxelGap;
	int32_t atDim = relevantDims() - 1;

	switch( atDim ) { // init offset with the given direction
	case readDim :
		offset = this->propertyValue( "readVec" )->castTo<util::fvector4>();
		break;
	case phaseDim:
		offset = this->propertyValue( "phaseVec" )->castTo<util::fvector4>();
		break;
	case sliceDim:

		if( this->hasProperty( "sliceVec" ) ) {
			offset = this->propertyValue( "sliceVec" )->castTo<util::fvector4>();
		} else {
			const util::fvector4 read = this->propertyValue( "readVec" )->castTo<util::fvector4>();
			const util::fvector4 phase = this->propertyValue( "phaseVec" )->castTo<util::fvector4>();
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
		util::fvector4 &orig = ref->propertyValue( "indexOrigin" )->castTo<util::fvector4>();
		uint32_t &acq = ref->propertyValue( "acquisitionNumber" )->castTo<uint32_t>();
		orig = orig + indexOriginOffset * ( float )cnt;
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
	const util::FixedVector<size_t, dims> wholesize = getSizeAsVector();
	util::FixedVector<size_t, dims> spliceSize;
	spliceSize.fill( 1 ); //init size of one chunk-splice to 1x1x1x1
	//copy the relevant dimensional sizes from wholesize (in case of sliceDim we copy only the first two elements of wholesize - making slices)
	spliceSize.copyFrom( &wholesize[0], &wholesize[atDim] );
	//get the spliced TypePtr's (the volume of the requested dims is the split-size - in case of sliceDim it is rows*columns)
	const TypePtrList pointers = this->getTypePtrBase().splice( spliceSize.product() );
	//create new Chunks from this TypePtr's
	BOOST_FOREACH( TypePtrList::const_reference ref, pointers ) {
		boost::shared_ptr<Chunk> spliced( new Chunk( ref, spliceSize[0], spliceSize[1], spliceSize[2], spliceSize[3] ) );
		static_cast<util::PropertyMap &>( *spliced ) = static_cast<const util::PropertyMap &>( *this ); //copy the metadate of ref
		ret.push_back( spliced ); // store splice for return
	}
	return ret;
}

const size_t Chunk::useCount() const
{
	return getTypePtrBase().useCount();
}

}
}

