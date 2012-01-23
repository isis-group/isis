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
#include <boost/scoped_ptr.hpp>

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
	util::vector4<size_t> newSize = getSizeAsVector();

	if ( nrOfColumns )newSize[0] = nrOfColumns;

	if ( nrOfRows )newSize[1] = nrOfRows;

	if ( nrOfSlices )newSize[2] = nrOfSlices;

	if ( nrOfTimesteps )newSize[3] = nrOfTimesteps;

	const ValuePtrReference cloned( getValuePtrBase().cloneToNew( newSize.product() ) );
	return Chunk( cloned, newSize[0], newSize[1], newSize[2], newSize[3] );
}

bool Chunk::convertToType( short unsigned int ID, scaling_pair scaling )
{
	//get a converted ValuePtr (will be a cheap copy if no conv was needed)
	ValuePtrReference newPtr = asValuePtrBase().convertByID( ID, scaling );

	if( newPtr.isEmpty() ) // if the reference is empty the conversion failed
		return false;
	else
		static_cast<ValuePtrReference &>( *this ) = newPtr; // otherwise replace my own ValuePtr with the new one

	return true;
}

size_t Chunk::bytesPerVoxel()const
{
	return getValuePtrBase().bytesPerElem();
}
std::string Chunk::getTypeName()const
{
	return getValuePtrBase().getTypeName();
}
unsigned short Chunk::getTypeID()const
{
	return getValuePtrBase().getTypeID();
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
			<< "Copy start " << util::vector4<size_t>( source_start )
			<< " is out of range (" << getSizeAsString() << ") at the source chunk";
	LOG_IF( ! isInRange( source_end ), Debug, error )
			<< "Copy end " << util::vector4<size_t>( source_end )
			<< " is out of range (" << getSizeAsString() << ") at the source chunk";
	LOG_IF( ! dst.isInRange( destination ), Debug, error )
			<< "Index " << util::vector4<size_t>( destination )
			<< " is out of range (" << getSizeAsString() << ") at the destination chunk";
	const size_t sstart = getLinearIndex( source_start );
	const size_t send = getLinearIndex( source_end );
	const size_t dstart = dst.getLinearIndex( destination );
	getValuePtrBase().copyRange( sstart, send, *dst, dstart );
}

size_t Chunk::compareRange( const size_t source_start[], const size_t source_end[], const Chunk &dst, const size_t destination[] ) const
{
	LOG_IF( ! isInRange( source_start ), Debug, error )
			<< "memcmp start " << util::vector4<size_t>( source_start )
			<< " is out of range (" << getSizeAsString() << ") at the first chunk";
	LOG_IF( ! isInRange( source_end ), Debug, error )
			<< "memcmp end " << util::vector4<size_t>( source_end )
			<< " is out of range (" << getSizeAsString() << ") at the first chunk";
	LOG_IF( ! dst.isInRange( destination ), Debug, error )
			<< "Index " << util::vector4<size_t>( destination )
			<< " is out of range (" << getSizeAsString() << ") at the second chunk";
	LOG( Debug, verbose_info )
			<< "Comparing range from " << util::vector4<size_t>( source_start ) << " to " << util::vector4<size_t>( source_end )
			<< " and " << util::vector4<size_t>( destination );
	const size_t sstart = getLinearIndex( source_start );
	const size_t send = getLinearIndex( source_end );
	const size_t dstart = dst.getLinearIndex( destination );
	return getValuePtrBase().compare( sstart, send, dst.getValuePtrBase(), dstart );
}
size_t Chunk::compare( const isis::data::Chunk &dst ) const
{
	if( getSizeAsVector() == dst.getSizeAsVector() )
		return getValuePtrBase().compare( 0, getVolume() - 1, dst.getValuePtrBase(), 0 );
	else
		return std::max( getVolume(), dst.getVolume() );
}


std::pair<util::ValueReference, util::ValueReference> Chunk::getMinMax ( ) const
{
	return getValuePtrBase().getMinMax();
}

scaling_pair Chunk::getScalingTo( unsigned short typeID, autoscaleOption scaleopt )const
{
	return getValuePtrBase().getScalingTo( typeID, scaleopt );
}
scaling_pair Chunk::getScalingTo( unsigned short typeID, const std::pair<util::ValueReference, util::ValueReference> &minmax, autoscaleOption scaleopt )const
{
	return getValuePtrBase().getScalingTo( typeID, minmax, scaleopt );
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

	LOG( Debug, info ) << "Splicing chunk at dimenstion " << atDim + 1 << " with indexOrigin stride " << indexOriginOffset << " and acquisitionNumberStride " << acquisitionNumberStride;
	std::list<Chunk> ret = splice( ( dimensions )atDim ); // do low level splice - get the chunklist

	std::list<Chunk>::iterator it = ret.begin();
	it++;// skip the first one

	for( size_t cnt = 1; it != ret.end(); it++, cnt++ ) { // adapt some metadata in them
		util::fvector4 &orig = it->propertyValue( "indexOrigin" )->castTo<util::fvector4>();

		if( orig == ret.front().getPropertyAs<util::fvector4>( "indexOrigin" ) ) { // fix pos if its the same as for the first
			LOG( Debug, verbose_info ) << "Origin was " << orig << " will be moved by " << indexOriginOffset << "*"  << cnt;
			orig = orig + indexOriginOffset * ( float )cnt;
		}

		uint32_t &acq = it->propertyValue( "acquisitionNumber" )->castTo<uint32_t>();

		if( acq == ret.front().getPropertyAs<uint32_t>( "acquisitionNumber" ) ) {
			LOG( Debug, verbose_info ) << "acquisitionNumber was " << acq << " will be moved by " << acquisitionNumberStride << "*"  << cnt;
			acq += acquisitionNumberStride * cnt; //@todo this might cause trouble if we try to insert this chunks into an image
		}
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

	const util::PropertyMap::KeyList lists = this->findLists();
	size_t list_idx = 0;

	//create new Chunks from this ValuePtr's
	BOOST_FOREACH( ValuePtrList::const_reference ref, pointers ) {
		ret.push_back( Chunk( ref, spliceSize[0], spliceSize[1], spliceSize[2], spliceSize[3] ) ); //@todo make sure this is only one copy-operation
		static_cast<util::PropertyMap &>( ret.back() ) = static_cast<const util::PropertyMap &>( *this ); // copy all props into the splices
		BOOST_FOREACH( const util::PropertyMap::KeyType & key, lists ) { // override list-entries in the splices with their respective entries
			ret.back().propertyValue( key ) = this->propertyValueAt( key, list_idx );
		}
		//      std::cout << static_cast<util::PropertyMap &>( ret.back() ) << std::endl;
		list_idx++;
	}
	return ret;
}

size_t Chunk::useCount() const
{
	return getValuePtrBase().useCount();
}

void Chunk::swapAlong( const dimensions dim ) const
{
	const size_t elSize = bytesPerVoxel();
	const util::vector4<size_t> whole_size = getSizeAsVector();
	const util::vector4<size_t> outer_size = whole_size;

	boost::shared_ptr<uint8_t> swap_ptr = boost::shared_static_cast<uint8_t>( get()->getRawAddress() );
	uint8_t *swap_start = swap_ptr.get();
	const uint8_t *const swap_end = swap_start + whole_size.product() * elSize;

	size_t block_volume = whole_size.product();

	for( int i = data::timeDim; i >= dim; i-- ) {
		assert( ( block_volume % whole_size[i] ) == 0 );
		block_volume /= whole_size[i];
	}

	assert( block_volume );
	block_volume *= elSize;
	const size_t swap_volume = block_volume * whole_size[dim];
	const boost::scoped_array<uint8_t> buff( new uint8_t[ block_volume ] );

	//iterate over all swap-volumes
	for( ; swap_start < swap_end; swap_start += swap_volume ) { //outer loop
		// swap each block with the one at the oppsite end of the swap_volume
		uint8_t *a = swap_start; //first block
		uint8_t *b = swap_start + swap_volume - block_volume; //last block within the swap-volume

		for( ; a < b; a += block_volume, b -= block_volume ) { // grow a, shrink b (inner loop)
			memcpy( buff.get(), a, block_volume );
			memcpy( a, b, block_volume );
			memcpy( b, buff.get(), block_volume );
		}

	}
}

util::PropertyValue &Chunk::propertyValueAt( const util::PropertyMap::KeyType &key, size_t at )
{
	std::vector< util::PropertyValue > &vec = propertyValueVec( key );
	const size_t cSize = getSizeAsVector()[getRelevantDims() - 1];

	if( vec.size() != cSize ) {
		LOG( Debug, info ) << "Resizing sub-property " << key << " to size of the chunk (" << cSize  << ")";
		vec.resize( cSize );
	}

	return vec.at( at );
}
const util::PropertyValue &Chunk::propertyValueAt( const util::PropertyMap::KeyType &key, size_t at ) const
{
	const std::vector< util::PropertyValue > &vec = propertyValueVec( key );
	const size_t cSize = getSizeAsVector()[getRelevantDims() - 1];
	LOG_IF( vec.size() != cSize, Debug, warning ) << "Sub-property " << key << " does not have the size of the chunk (" << cSize  << ")";
	return vec.at( at );
}

Chunk::value_iterator Chunk::begin()
{
	return asValuePtrBase().beginGeneric();
}

Chunk::value_iterator Chunk::end()
{
	return asValuePtrBase().endGeneric();
}
Chunk::const_value_iterator Chunk::begin()const
{
	return getValuePtrBase().beginGeneric();
}

Chunk::const_value_iterator Chunk::end()const
{
	return getValuePtrBase().endGeneric();
}

}
}
