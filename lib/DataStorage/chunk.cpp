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
	LOG_IF( NDimensional<4>::volume() == 0, Debug, warning )
			<< "Size " << fourthDim << "|" << thirdDim << "|" << secondDim << "|" << firstDim << " is invalid";
}

ChunkBase::~ChunkBase() { }

}

Chunk::Chunk( const _internal::TypePtrBase::Reference &src, size_t firstDim, size_t secondDim, size_t thirdDim, size_t fourthDim ):
	_internal::ChunkBase( firstDim, secondDim, thirdDim, fourthDim ),
	_internal::TypePtrBase::Reference( src )
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

	const _internal::TypePtrBase::Reference
	cloned( get()->cloneToMem( newSize.product() ) );
	return Chunk( cloned, newSize[0], newSize[1], newSize[2], newSize[3] );
}
Chunk Chunk::copyToMem()const
{
	Chunk ret( *this );
	static_cast<_internal::TypePtrBase::Reference &>( ret ) = get()->copyToMem();
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

void Chunk::copyLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, secondDimD, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, secondDimD, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}
void Chunk::copySlice( size_t thirdDimS, size_t fourthDimS, Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, 0, thirdDimD, fourthDimD};
	const size_t idx3[] = {sizeToVector()[0] - 1, sizeToVector()[1] - 1, thirdDimD, fourthDimD};
	copyRange( idx1, idx2, dst, idx3 );
}

void Chunk::copyRange( const size_t source_start[], const size_t source_end[], Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! rangeCheck( source_start ), Debug, isis::error )
			<< "Copy start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << sizeToString() << ") at the source chunk";
	LOG_IF( ! rangeCheck( source_end ), Debug, isis::error )
			<< "Copy end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << sizeToString() << ") at the source chunk";
	LOG_IF( ! dst.rangeCheck( destination ), Debug, isis::error )
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

size_t Chunk::cmpRange( size_t start, size_t end, const isis::data::Chunk &dst, size_t destination ) const
{
	return get()->cmp( start, end, *dst, destination );
}
size_t Chunk::cmpRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! rangeCheck( source_start ), Debug, isis::error )
			<< "memcmp start " << util::FixedVector<size_t, 4>( source_start )
			<< " is out of range (" << sizeToString() << ") at the first chunk";
	LOG_IF( ! rangeCheck( source_end ), Debug, isis::error )
			<< "memcmp end " << util::FixedVector<size_t, 4>( source_end )
			<< " is out of range (" << sizeToString() << ") at the first chunk";
	LOG_IF( ! dst.rangeCheck( destination ), Debug, isis::error )
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
void Chunk::getMinMax ( util::_internal::TypeBase::Reference &min, util::_internal::TypeBase::Reference &max ) const
{
	return operator*().getMinMax( min, max );
}
Chunk &Chunk::operator=( const isis::data::Chunk &ref )
{
	_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
	_internal::TypePtrBase::Reference::operator=( static_cast<const _internal::TypePtrBase::Reference &>( ref ) ); // copy the reference of ref's data
	return *this;
}

void Chunk::transformCoords(boost::numeric::ublas::matrix<float> transform)
{

	// create boost::numeric::ublast matrix from orientation vectors
	boost::numeric::ublas::matrix<float> orient(3,3);
	util::fvector4 read = getProperty<util::fvector4>("readVec");
	util::fvector4 phase = getProperty<util::fvector4>("phaseVec");
	util::fvector4 slice = getProperty<util::fvector4>("sliceVec");
	util::fvector4 origin = getProperty<util::fvector4>("indexOrigin");

	// copy orientation vectors into matrix columns
	// readVec
	for(unsigned i = 0;i<3;i++){
		orient(i,0) = read[i];
	}

	// phaseVec
	for(unsigned i = 0;i<3;i++){
		orient(i,1) = phase[i];
	}

	// sliceVec
	for(unsigned i = 0;i<3;i++){
		orient(i,2) = slice[i];
	}

	// copy index origin
	boost::numeric::ublas::vector<float> o(3);
	for (unsigned i = 0;i < 3;i++){
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
	for(unsigned i=0;i<3;i++){
		origin[i] = new_o(i);
	}

	// readVec
	for(unsigned i=0;i<3;i++){
		read[i] = new_orient(i,0);
	}

	// phaseVec
	for(unsigned i=0;i<3;i++){
		phase[i] = new_orient(i,1);
	}

	// sliceVec
	for(unsigned i=0;i<3;i++){
		slice[i] = new_orient(i,2);
	}

	setProperty<util::fvector4>("indexOrigin",origin);
	setProperty<util::fvector4>("readVec",read);
	setProperty<util::fvector4>("phaseVec",phase);
	setProperty<util::fvector4>("sliceVec",slice);

}

}
}

