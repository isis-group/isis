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

#include "chunk.hpp"
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

	if ( !NDimensional<4>::volume() )
		LOG( Debug, warning )
				<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid";
}

ChunkBase::~ChunkBase() { }

}

Chunk::Chunk( const util::_internal::TypePtrBase::Reference &src, size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim ):
	_internal::ChunkBase( firstDim, secondDim, thirdDim, fourthDim ),
	util::_internal::TypePtrBase::Reference( src )
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

	const util::_internal::TypePtrBase::Reference
	cloned( get()->cloneToMem( newSize.product() ) );
	return Chunk( cloned, newSize[0], newSize[1], newSize[2], newSize[3] );
}
Chunk Chunk::copyToMem()const
{
	Chunk ret( *this );
	static_cast<util::_internal::TypePtrBase::Reference&>( ret ) = get()->copyToMem();
	return ret;
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

void Chunk::copyLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, Chunk& dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, secondDimD, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, secondDimD, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}
void Chunk::copySlice( size_t thirdDimS, size_t fourthDimS, Chunk& dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, 0, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, sizeToVector()[1] - 1, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}

void Chunk::copyRange( const size_t source_start[], const size_t source_end[], Chunk& dst, const size_t destination[] ) const
{
	LOG_IF( not rangeCheck( source_start ), Debug, isis::error )
			<< "Copy start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << sizeToString() << ") at the source chunk";
	LOG_IF( not rangeCheck( source_end ), Debug, isis::error )
			<< "Copy end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << sizeToString() << ") at the source chunk";
	LOG_IF( not dst.rangeCheck( destination ), Debug, isis::error )
			<< "Index " << util::FixedVector<size_t, 4>( destination )
			<< " is out of range (" << sizeToString() << ") at the destination chunk";
	LOG( Debug, isis::verbose_info )
			<< "Copying range from " << util::FixedVector<size_t, 4>( source_start ) << " to " << util::FixedVector<size_t, 4>( source_end )
			<< " to " << util::FixedVector<size_t, 4>( destination );
	const size_t sstart = dim2Index( source_start );
	const size_t send = dim2Index( source_end );
	const size_t dstart = dst.dim2Index( destination );
	get()->copyRange( sstart, send, *dst, dstart );
}

size_t Chunk::cmpRange( size_t start, size_t end, const isis::data::Chunk& dst, size_t destination ) const
{
	return get()->cmp( start, end, *dst, destination );
}
size_t Chunk::cmpRange( const size_t source_start[], const size_t source_end[], const Chunk& dst, const size_t destination[] ) const
{
	LOG_IF( not rangeCheck( source_start ), Debug, isis::error )
			<< "memcmp start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << sizeToString() << ") at the first chunk";
	LOG_IF( not rangeCheck( source_end ), Debug, isis::error )
			<< "memcmp end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << sizeToString() << ") at the first chunk";
	LOG_IF( not dst.rangeCheck( destination ), Debug, isis::error )
			<< "Index " << util::FixedVector<size_t, 4>( destination )
			<< " is out of range (" << sizeToString() << ") at the second chunk";
	LOG( Debug, isis::verbose_info )
			<< "Comparing range from " << util::FixedVector<size_t, 4>( source_start ) << " to " << util::FixedVector<size_t, 4>( source_end )
			<< " and " << util::FixedVector<size_t, 4>( destination );
	const size_t sstart = dim2Index( source_start );
	const size_t send = dim2Index( source_end );
	const size_t dstart = dst.dim2Index( destination );
	return cmpRange( sstart, send, dst, dstart );
}
size_t Chunk::cmpLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, const Chunk& dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, secondDimD, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, secondDimD, thirdDimD, fourthDimD};
	return cmpRange( idx1, idx2, dst, idx3 );
}
size_t Chunk::cmpSlice( size_t thirdDimS, size_t fourthDimS, const Chunk& dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, 0, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, sizeToVector()[1] - 1, thirdDimD, fourthDimD};
	return cmpRange( idx1, idx2, dst, idx3 );
}
void Chunk::getMinMax ( util::_internal::TypeBase& min, util::_internal::TypeBase& max, bool init ) const
{
	return operator*().getMinMax( min, max, init );
}
Chunk& Chunk::operator=( const isis::data::Chunk& ref )
{
	_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase&>( ref ) ); //copy the metadate of ref
	util::_internal::TypePtrBase::Reference::operator=( static_cast<const util::_internal::TypePtrBase::Reference&>( ref ) ); // copy the reference of ref's data
}

}
}

