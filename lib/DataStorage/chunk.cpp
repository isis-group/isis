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
Chunk Chunk::copyToMem()const
{
	Chunk ret( *this );
	static_cast<TypePtrReference &>( ret ) = get()->copyToMem();
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
void Chunk::getMinMax ( util::TypeReference& min, util::TypeReference& max ) const
{
	return operator*().getMinMax( min, max );
}
Chunk &Chunk::operator=( const isis::data::Chunk &ref )
{
	_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
	TypePtrReference::operator=( static_cast<const TypePtrReference &>( ref ) ); // copy the reference of ref's data
	return *this;
}

void Chunk::transformCoords( boost::numeric::ublas::matrix<float> transform )
{
	// create boost::numeric::ublast matrix from orientation vectors
	boost::numeric::ublas::matrix<float> orient( 3, 3 );
	util::fvector4 read = getProperty<util::fvector4>( "readVec" );
	util::fvector4 phase = getProperty<util::fvector4>( "phaseVec" );
	util::fvector4 slice = getProperty<util::fvector4>( "sliceVec" );
	util::fvector4 origin = getProperty<util::fvector4>( "indexOrigin" );

	// copy orientation vectors into matrix columns
	// readVec
	for( unsigned i = 0; i < 3; i++ ) {
		orient( i, 0 ) = read[i];
	}

	// phaseVec
	for( unsigned i = 0; i < 3; i++ ) {
		orient( i, 1 ) = phase[i];
	}

	// sliceVec
	for( unsigned i = 0; i < 3; i++ ) {
		orient( i, 2 ) = slice[i];
	}

	// copy index origin
	boost::numeric::ublas::vector<float> o( 3 );

	for ( unsigned i = 0; i < 3; i++ ) {
		o( i ) = origin[i];
	}

	// since the orientation matrix is 3x3 orthogonal matrix it holds that
	// orient * orient^T = I, where I is the identity matrix.
	// calculate new orientation matrix --> O_new = O * T
	boost::numeric::ublas::matrix<float> new_orient =
		boost::numeric::ublas::prod( orient, transform );
	// transform index origin into new coordinate space.
	// o_new -> O_new * (O^-1 * o)
	boost::numeric::ublas::vector<float> new_o =
		boost::numeric::ublas::prod( new_orient,
									 ( boost::numeric::ublas::vector<float> )boost::numeric::ublas::prod(
										 ( boost::numeric::ublas::matrix<float> )boost::numeric::ublas::trans( orient ), o ) );

	// write origin back to attributes
	for( unsigned i = 0; i < 3; i++ ) {
		origin[i] = new_o( i );
	}

	// readVec
	for( unsigned i = 0; i < 3; i++ ) {
		read[i] = new_orient( i, 0 );
	}

	// phaseVec
	for( unsigned i = 0; i < 3; i++ ) {
		phase[i] = new_orient( i, 1 );
	}

	// sliceVec
	for( unsigned i = 0; i < 3; i++ ) {
		slice[i] = new_orient( i, 2 );
	}

	setProperty<util::fvector4>( "indexOrigin", origin );
	setProperty<util::fvector4>( "readVec", read );
	setProperty<util::fvector4>( "phaseVec", phase );
	setProperty<util::fvector4>( "sliceVec", slice );
}

ChunkList Chunk::autoSplice ( int32_t acquisitionNumberOffset )
{
	if (!valid()) {
		LOG(Runtime,error) << "Cannot splice invalid Chunk (missing properties are " << this->getMissing() << ")";
		return ChunkList();
	}
	
	util::fvector4 offset;
	const util::fvector4 voxelSize = propertyValue( "voxelSize" )->cast_to_Type<util::fvector4>();
	util::fvector4 voxelGap;
	if(hasProperty("voxelGap"))
		voxelGap = propertyValue( "voxelGap" )->cast_to_Type<util::fvector4>();

	const util::fvector4 distance = voxelSize + voxelGap;

	int32_t atDim=relevantDims()-1;

	LOG(Runtime,info) << "Splicing chunk at dimenstion " << atDim+1;
	
	switch( atDim ) { // init offset with the given direction
	case readDim :
		offset = this->propertyValue( "readVec" )->cast_to_Type<util::fvector4>();
		break;
	case phaseDim:
		offset = this->propertyValue( "phaseVec" )->cast_to_Type<util::fvector4>();
		break;
	case sliceDim:

		if( this->hasProperty( "sliceVec" ) )
			offset = this->propertyValue( "sliceVec" )->cast_to_Type<util::fvector4>();
		else {
			const util::fvector4 read = this->propertyValue( "readVec" )->cast_to_Type<util::fvector4>();
			const util::fvector4 phase = this->propertyValue( "phaseVec" )->cast_to_Type<util::fvector4>();
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

	ChunkList ret=splice( (dimensions)atDim ); // do low level splice - get the chunklist

	// prepare some attributes
	assert( util::fuzzyEqual<float>( offset.sqlen(), 1 ) ); // it should be norm here
	const util::fvector4 indexOriginOffset=offset * distance[atDim];
	size_t cnt=0;
		
	BOOST_FOREACH(ChunkList::reference ref,ret){ // adapt some metadata in them
		util::fvector4 &orig = ref.propertyValue( "indexOrigin" )->cast_to_Type<util::fvector4>();
		int32_t &acq=propertyValue( "acquisitionNumber" )->cast_to_Type<int32_t>();
		
		orig= orig + indexOriginOffset * cnt;
		acq= acquisitionNumberOffset + cnt;//@todo this might cause trouble if we try to insert this chunks into an image
		
		cnt++;
	}
	
	return ret;
}

ChunkList Chunk::splice ( dimensions atDim )
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
	const TypePtrList pointers = this->asTypePtrBase().splice( spliceSize.product() );
	
	//create new Chunks from this TypePtr's
	BOOST_FOREACH( TypePtrList::const_reference ref, pointers ) {
		Chunk spliced( ref, spliceSize[0], spliceSize[1], spliceSize[2], spliceSize[3] );
		static_cast<util::PropMap &>( spliced ) = static_cast<util::PropMap &>( *this ); //copy the metadate of ref
		ret.push_back( spliced ); // store splice for return
	}
	return ret;
}
bool Chunk::swapAlong(  Chunk &dst, const size_t dim, bool convertTransform ) const
{
	size_t dims[] = { dimSize( 0 ), dimSize( 1 ), dimSize( 2 ), dimSize( 3 ) };

	if ( get()->swapAlong( *dst, dim, dims ) ) {
		const isis::util::PropMap &tmpMap( *this );
		static_cast<PropMap &>( dst ) = tmpMap;

		if ( convertTransform ) {
			util::fvector4 read = getProperty<util::fvector4>( "readVec" );
			util::fvector4 phase = getProperty<util::fvector4>( "phaseVec" );
			util::fvector4 slice = getProperty<util::fvector4>( "sliceVec" );
			util::fvector4 origin = getProperty<util::fvector4>( "indexOrigin" );
			read[dim] = -read[dim];
			phase[dim] = -phase[dim];
			slice[dim] = -slice[dim];
			origin[dim] = -origin[dim];
			dst.setProperty<util::fvector4>( "readVec", read );
			dst.setProperty<util::fvector4>( "phaseVec", phase );
			dst.setProperty<util::fvector4>( "sliceVec", slice );
			dst.setProperty<util::fvector4>( "indexOrigin", origin );
		}

		return 1;
	} else return 0;
}

}
}

