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

#include "chunk.hpp"
#include <boost/foreach.hpp>
#include <limits>

namespace isis
{
namespace data
{
namespace _internal
{

ChunkBase::ChunkBase ( size_t nrOfColumns, size_t nrOfRows, size_t nrOfSlices, size_t nrOfTimesteps )
{
	const size_t idx[] = {nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps};
	init( idx );
	addNeededFromString( neededProperties );
	LOG_IF( NDimensional<4>::getVolume() == 0, Debug, warning )
			<< "Size " << nrOfTimesteps << "|" << nrOfSlices << "|" << nrOfRows << "|" << nrOfColumns << " is invalid";
}

ChunkBase::~ChunkBase() { }

}

Chunk::Chunk( const ValuePtrReference &src, size_t nrOfColumns, size_t nrOfRows, size_t nrOfSlices, size_t nrOfTimesteps ):
	_internal::ChunkBase( nrOfColumns, nrOfRows, nrOfSlices, nrOfTimesteps ),
	ValuePtrReference( src )
{
	assert( ( *this )->getLength() == getVolume() );
}

Chunk Chunk::cloneToNew( size_t nrOfColumns, size_t nrOfRows, size_t nrOfSlices, size_t nrOfTimesteps )const
{
	util::FixedVector<size_t, 4> newSize = getSizeAsVector();

	if ( nrOfColumns )newSize[0] = nrOfColumns;

	if ( nrOfRows )newSize[1] = nrOfRows;

	if ( nrOfSlices )newSize[2] = nrOfSlices;

	if ( nrOfTimesteps )newSize[3] = nrOfTimesteps;

	const ValuePtrReference cloned( get()->cloneToNew( newSize.product() ) );
	return Chunk( cloned, newSize[0], newSize[1], newSize[2], newSize[3] );
}

bool Chunk::convertToType( short unsigned int ID )
{
	if( getTypeID() != ID ) {
		return convertToType( ID, getScalingTo( ID ) );
	}

	return true;
}

bool Chunk::convertToType( short unsigned int ID, const scaling_pair &scaling )
{
	static const util::Value<uint8_t> one( 1 );
	static const util::Value<uint8_t> zero( 0 );

	if( getTypeID() != ID || !( scaling.first->eq( one ) && scaling.second->eq( zero ) ) ) { // if its not the same type - replace the internal ValuePtr by a new returned from ValuePtrBase::copyToNewById
		ValuePtrReference newPtr = getValuePtrBase().copyToNewByID( ID, scaling ); // create a new ValuePtr of type id and store it in a ValuePtrReference

		if( newPtr.isEmpty() ) // if the reference is empty the conversion failed
			return false;

		static_cast<ValuePtrReference &>( *this ) = newPtr; // otherwise replace my own ValuePtr with the new one
	}

	return true;
}

size_t Chunk::bytesPerVoxel()const
{
	return get()->bytesPerElem();
}
std::string Chunk::getTypeName()const
{
	return get()->getTypeName();
}
unsigned short Chunk::getTypeID()const
{
	return get()->getTypeID();
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

size_t Chunk::compareRange( size_t flat_start, size_t flat_end, const Chunk &dst, size_t destination ) const
{
	return get()->compare( flat_start, flat_end, *dst, destination );
}
size_t Chunk::compareRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] ) const
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
	return compareRange( sstart, send, dst, dstart );
}
size_t Chunk::compareLine( size_t secondDimS, size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t secondDimD, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, secondDimS, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, secondDimD, thirdDimD, fourthDimD};
	const size_t idx3[] = {getSizeAsVector()[0] - 1, secondDimD, thirdDimD, fourthDimD};
	return compareRange( idx1, idx2, dst, idx3 );
}
size_t Chunk::compareSlice( size_t thirdDimS, size_t fourthDimS, const Chunk &dst, size_t thirdDimD, size_t fourthDimD ) const
{
	const size_t idx1[] = {0, 0, thirdDimS, fourthDimS};
	const size_t idx2[] = {0, 0, thirdDimD, fourthDimD};
	const size_t idx3[] = {getSizeAsVector()[0] - 1, getSizeAsVector()[1] - 1, thirdDimD, fourthDimD};
	return compareRange( idx1, idx2, dst, idx3 );
}
std::pair<util::ValueReference, util::ValueReference> Chunk::getMinMax ( ) const
{
	return operator*().getMinMax();
}

scaling_pair Chunk::getScalingTo( unsigned short typeID, autoscaleOption scaleopt )const
{
	std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
	return operator*().getScalingTo( typeID, *minmax.first, *minmax.second, scaleopt );
}
scaling_pair Chunk::getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt )const
{
	return operator*().getScalingTo( typeID, minmax, scaleopt );
}
scaling_pair Chunk::getScalingTo( unsigned short typeID, const util::_internal::ValueBase &min, const util::_internal::ValueBase &max, autoscaleOption scaleopt )const
{
	return operator*().getScalingTo( typeID, min, max, scaleopt );
}

Chunk &Chunk::operator=( const Chunk &ref )
{
	_internal::ChunkBase::operator=( static_cast<const _internal::ChunkBase &>( ref ) ); //copy the metadate of ref
	ValuePtrReference::operator=( static_cast<const ValuePtrReference &>( ref ) ); // copy the reference of ref's data
	return *this;
}

std::list<Chunk> Chunk::autoSplice ( uint32_t acquisitionNumberStride )const
{
	if ( !isValid() ) {
		LOG( Runtime, error ) << "Cannot splice invalid Chunk (missing properties are " << this->getMissing() << ")";
		return std::list<Chunk>();
	}

	util::fvector4 offset;
	const util::fvector4 voxelSize = propertyValue( "voxelSize" )->castTo<util::fvector4>();
	util::fvector4 voxelGap;

	if( hasProperty( "voxelGap" ) )
		voxelGap = propertyValue( "voxelGap" )->castTo<util::fvector4>();

	const util::fvector4 distance = voxelSize + voxelGap;
	size_t atDim = getRelevantDims() - 1;

	switch( atDim ) { // init offset with the given direction
	case rowDim :
		offset = this->propertyValue( "rowVec" )->castTo<util::fvector4>();
		break;
	case columnDim:
		offset = this->propertyValue( "columnVec" )->castTo<util::fvector4>();
		break;
	case sliceDim:

		if( this->hasProperty( "sliceVec" ) ) {
			offset = this->propertyValue( "sliceVec" )->castTo<util::fvector4>();
		} else {
			const util::fvector4 row = this->propertyValue( "rowVec" )->castTo<util::fvector4>();
			const util::fvector4 column = this->propertyValue( "columnVec" )->castTo<util::fvector4>();
			assert( util::fuzzyEqual<float>( row.sqlen(), 1 ) );
			assert( util::fuzzyEqual<float>( column.sqlen(), 1 ) );
			offset[0] = row[1] * column[2] - row[2] * column[1];
			offset[1] = row[2] * column[0] - row[0] * column[2];
			offset[2] = row[0] * column[1] - row[1] * column[0];
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
	std::list<Chunk> ret = splice( ( dimensions )atDim ); // do low level splice - get the chunklist
	BOOST_FOREACH( Chunk & ref, ret ) { // adapt some metadata in them
		util::fvector4 &orig = ref.propertyValue( "indexOrigin" )->castTo<util::fvector4>();
		uint32_t &acq = ref.propertyValue( "acquisitionNumber" )->castTo<uint32_t>();
		orig = orig + indexOriginOffset * ( float )cnt;
		acq += acquisitionNumberStride * cnt; //@todo this might cause trouble if we try to insert this chunks into an image
		cnt++;
	}
	return ret;
}

std::list<Chunk> Chunk::splice ( dimensions atDim )const
{
	std::list<Chunk> ret;
	//@todo should be locking
	typedef std::vector<ValuePtrReference> ValuePtrList;
	const util::FixedVector<size_t, dims> wholesize = getSizeAsVector();
	util::FixedVector<size_t, dims> spliceSize;
	spliceSize.fill( 1 ); //init size of one chunk-splice to 1x1x1x1
	//copy the relevant dimensional sizes from wholesize (in case of sliceDim we copy only the first two elements of wholesize - making slices)
	spliceSize.copyFrom( &wholesize[0], &wholesize[atDim] );
	//get the spliced ValuePtr's (the volume of the requested dims is the split-size - in case of sliceDim it is rows*columns)
	const ValuePtrList pointers = this->getValuePtrBase().splice( spliceSize.product() );
	//create new Chunks from this ValuePtr's
	BOOST_FOREACH( ValuePtrList::const_reference ref, pointers ) {
		ret.push_back( Chunk( ref, spliceSize[0], spliceSize[1], spliceSize[2], spliceSize[3] ) ); //@todo make sure zhis is only one copy-operation
		static_cast<util::PropertyMap &>( ret.back() ) = static_cast<const util::PropertyMap &>( *this ); //copy my metadate into all spliced
	}
	return ret;
}

size_t Chunk::useCount() const
{
	return getValuePtrBase().useCount();
}

void Chunk::swapAlong( const dimensions dim ) const
{
	const size_t elSize=bytesPerVoxel();
	const util::FixedVector<size_t,4> whole_size=getSizeAsVector();
	size_t block_volume=whole_size.product();
	for(int i=data::timeDim;i>=dim;i--){
		assert((block_volume%getDimSize(i))==0);
		block_volume/=getDimSize(i);
	}

	assert(block_volume);
	block_volume*=elSize;
	std::auto_ptr<uint8_t> buff(static_cast<uint8_t*>(malloc(block_volume)));

	boost::shared_ptr<uint8_t> p=boost::shared_static_cast<uint8_t>(get()->getRawAddress().lock());
	uint8_t* a=p.get(); //first block
	uint8_t* b=a+getVolume()*bytesPerVoxel()-block_volume; //last block
	for(;a<b;a+=block_volume,b-=block_volume){ // grow a, shrink b 
		memcpy(buff.get(),a,block_volume);
		memcpy(a,b,block_volume);
		memcpy(b,buff.get(),block_volume);
	}
}
}
}
