//
// C++ Interface: image
//
// Description:
//
//
// Author: Enrico Reimer<reimer@cbs.mpg.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef IMAGE_H
#define IMAGE_H

#include "chunk.hpp"

#include <set>
#include <boost/shared_ptr.hpp>
#include <vector>
#include <boost/foreach.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/shared_array.hpp>
#include <stack>
#include <boost/none.hpp>
#include "sortedchunklist.hpp"
#include "common.hpp"

using boost::optional;
using boost::none;

namespace isis
{
namespace data
{
namespace _internal
{
/**
 * Generic iterator for voxels in Images.
 * It automatically jumps from chunk to Chunk.
 * It needs the chunks and the image to be there for to work properly (so don't delete the image, and dont reIndex it),
 * It assumes that all Chunks have the same size (which is a rule for Image as well, so this should be given)
 */
template<typename CHUNK_TYPE> class ImageIteratorTemplate: public std::iterator <
	std::random_access_iterator_tag,
	typename boost::mpl::if_<boost::is_const<CHUNK_TYPE>, typename CHUNK_TYPE::const_iterator, typename CHUNK_TYPE::iterator>::type::value_type,
	typename boost::mpl::if_<boost::is_const<CHUNK_TYPE>, typename CHUNK_TYPE::const_iterator, typename CHUNK_TYPE::iterator>::type::difference_type,
	typename boost::mpl::if_<boost::is_const<CHUNK_TYPE>, typename CHUNK_TYPE::const_iterator, typename CHUNK_TYPE::iterator>::type::pointer,
	typename boost::mpl::if_<boost::is_const<CHUNK_TYPE>, typename CHUNK_TYPE::const_iterator, typename CHUNK_TYPE::iterator>::type::reference
	>
{
protected:
	typedef typename boost::mpl::if_<boost::is_const<CHUNK_TYPE>, typename CHUNK_TYPE::const_iterator, typename CHUNK_TYPE::iterator>::type inner_iterator;
	typedef CHUNK_TYPE chunk_type;
	typedef ImageIteratorTemplate<CHUNK_TYPE> ThisType;

	//we have to use the non-const here, otherwise the iterator would not be convertible into const_iterator
	boost::shared_array<typename boost::remove_const<CHUNK_TYPE>::type *> chunks;

	size_t ch_idx, ch_cnt;
	inner_iterator current_it;
	typename inner_iterator::difference_type ch_len;

	typename inner_iterator::difference_type currentDist() const {
		if ( ch_idx >= ch_cnt )
			return 0; // if we're behind the last chunk assume we are at the "start" of the "end"-chunk
		else {
			const inner_iterator chit_begin = chunks[ch_idx]->begin(); // cast in a const_ or cast out a non existing one
			return std::distance ( chit_begin, current_it ); // so we use same iterators here
		}
	}
	friend class ImageIteratorTemplate<const CHUNK_TYPE>; //yes, I'm my own friend, sometimes :-) (enables the constructor below)
public:

	//will become additional constructor from non const if this is const, otherwise overrride the default copy contructor
	ImageIteratorTemplate ( const ImageIteratorTemplate<typename boost::remove_const<CHUNK_TYPE>::type > &src ) :
		chunks ( src.chunks ), ch_idx ( src.ch_idx ), ch_cnt( src.ch_cnt ),
		current_it ( src.current_it ),
		ch_len ( src.ch_len )
	{}

	// empty constructor
	ImageIteratorTemplate() : ch_idx ( 0 ), ch_cnt( 0 ), ch_len ( 0 ) {}


	// normal conytructor
	explicit ImageIteratorTemplate ( boost::shared_array<typename boost::remove_const<CHUNK_TYPE>::type *> &_chunks, size_t _ch_cnt ) :
		chunks ( _chunks ), ch_idx ( 0 ), ch_cnt( _ch_cnt ),
		current_it ( chunks[0]->begin() ),
		ch_len ( std::distance ( current_it, const_cast<CHUNK_TYPE *>( chunks[0] )->end() ) )
	{}

	ThisType &operator++() {
		return operator+= ( 1 );
	}
	ThisType &operator--() {
		return operator-= ( 1 );
	}

	ThisType operator++ ( int ) {
		ThisType tmp = *this;
		operator++();
		return tmp;
	}
	ThisType operator-- ( int ) {
		ThisType tmp = *this;
		operator--();
		return tmp;
	}

	typename inner_iterator::reference operator*() const {
		return current_it.operator * ();
	}
	typename inner_iterator::pointer  operator->() const {
		return current_it.operator->();
	}

	bool operator== ( const ThisType &cmp ) const {
		return ch_idx == cmp.ch_idx && current_it == cmp.current_it;
	}
	bool operator!= ( const ThisType &cmp ) const {
		return !operator== ( cmp );
	}

	bool operator> ( const ThisType &cmp ) const {
		return ch_idx > cmp.ch_idx || ( ch_idx == cmp.ch_idx && current_it > cmp.current_it );
	}
	bool operator< ( const ThisType &cmp ) const {
		return ch_idx < cmp.ch_idx || ( ch_idx == cmp.ch_idx && current_it < cmp.current_it );
	}


	bool operator>= ( const ThisType &cmp ) const {
		return operator> ( cmp ) || operator== ( cmp );
	}
	bool operator<= ( const ThisType &cmp ) const {
		return operator< ( cmp ) || operator== ( cmp );
	}

	typename inner_iterator::difference_type operator- ( const ThisType &cmp ) const {
		typename inner_iterator::difference_type dist = ( ch_idx - cmp.ch_idx ) * ch_len; // get the (virtual) distance from my current block to cmp's current block

		if ( ch_idx >= cmp.ch_idx ) { //if I'm beyond cmp add my current pos to the distance, and substract his
			dist += currentDist() - cmp.currentDist();
		} else {
			dist += cmp.currentDist() - currentDist();
		}

		return dist;
	}

	ThisType operator+ ( typename ThisType::difference_type n ) const {
		return ThisType ( *this ) += n;
	}
	ThisType operator- ( typename ThisType::difference_type n ) const {
		return ThisType ( *this ) -= n;
	}

	ThisType &operator+= ( typename inner_iterator::difference_type n ) {
		n += currentDist(); //start from current begin (add current_it-(begin of the current chunk) to n)
		assert ( ( n / ch_len + static_cast<typename ThisType::difference_type> ( ch_idx ) ) >= 0 );
		ch_idx += n / ch_len; //if neccesary jump to next chunk

		if ( ch_idx < ch_cnt )
			current_it = chunks[ch_idx]->begin() + n % ch_len; //set new current iterator in new chunk plus the "rest"
		else
			current_it = chunks[ch_cnt - 1]->end() ; //set current_it to the last chunks end iterator if we are behind it

		//@todo will break if ch_cnt==0

		return *this;
	}
	ThisType &operator-= ( typename inner_iterator::difference_type n ) {
		return operator+= ( -n );
	}

	typename ThisType::reference operator[] ( typename inner_iterator::difference_type n ) const throw( std::out_of_range ) {
		n += currentDist(); //start from current begin (add current_it-(begin of the current chunk) to n)
		assert ( (ch_idx + n / ch_len ) >= 0 );
		const size_t my_ch_idx = static_cast<size_t>(ch_idx + n / ch_len); //if neccesary jump to next chunk

		if ( my_ch_idx >= ch_cnt )
			throw std::out_of_range(
				std::string( "Image voxel index " ) + boost::lexical_cast<std::string>( ch_idx * ch_len + currentDist() ) + "+"
				+ boost::lexical_cast<std::string>( n - currentDist() )
				+ " out of range 0.." + boost::lexical_cast<std::string>( ch_cnt ) + "*"
				+ boost::lexical_cast<std::string>( ch_len ) + "-1"
			);

		return  *( chunks[my_ch_idx]->begin() + n % ch_len );
	}

};
}

/// Base class for operators used for foreachChunk
class ChunkOp : std::unary_function<Chunk &, bool>
{
public:
	virtual bool operator() ( Chunk &, util::vector4<size_t> posInImage ) = 0;
	virtual ~ChunkOp();
};

/// Main class for generic 4D-images
class Image:
	public _internal::NDimensional<4>,
	public util::PropertyMap
{
	dimensions minIndexingDim;
public:
	/**
	 * Enforce indexing to start at a given dimension.
	 * Normally indexing starts at the dimensionality of the inserted chunks.
	 * So, an Image of 2D-Chunks (slices) will start indexing at the 3rd dimension.
	 * If the dimension given here is bigger than the dimensionality of the chunks reindexing will override that and start indexing at the given dimension.
	 * E.g. setIndexingDim(timeDim) will enforce indexing of a Image of 10 30x30-slices at the time dimension resulting in an 30x30x1x10 image instead of an 30x30x10x1 image.
	 * If the indexing dimension is set after the Image was indexed it will be indexed again.
	 * \param d the minimal indexing dimension to be used
	 */
	void setIndexingDim ( dimensions d = rowDim );
	enum orientation {axial, reversed_axial, sagittal, reversed_sagittal, coronal, reversed_coronal};

	typedef _internal::ImageIteratorTemplate<Chunk> iterator;
	typedef _internal::ImageIteratorTemplate<const Chunk> const_iterator;
	typedef iterator::reference reference;
	typedef const_iterator::reference const_reference;
	static const char *neededProperties;
protected:
	_internal::SortedChunkList set;
	std::vector<boost::shared_ptr<Chunk> > lookup;
private:
	size_t chunkVolume;

	void deduplicateProperties();

	/**
	 * Get the pointer to the chunk in the internal lookup-table at position at.
	 * The Chunk will only have metadata which are unique to it - so it might be invalid
	 * (run join on it using the image as parameter to insert all non-unique-metadata).
	 */
	const boost::shared_ptr<Chunk> &chunkPtrAt ( size_t at ) const;

	/**
	 * Computes chunk- and voxel- indices.
	 * The returned chunk-index applies to the lookup-table (chunkAt), and the voxel-index to this chunk.
	 * Behaviour will be undefined if:
	 * - the image is not clean (not indexed)
	 * - the image is empty
	 * - the coordinates are not in the image
	 *
	 * Additionally an error will be sent if Debug is enabled.
	 * \returns a std::pair\<chunk-index,voxel-index\>
	 */
	inline std::pair<size_t, size_t> commonGet ( size_t first, size_t second, size_t third, size_t fourth ) const {
		const size_t idx[] = {first, second, third, fourth};
		LOG_IF ( ! clean, Debug, error )
				<< "Getting data from a non indexed image will result in undefined behavior. Run reIndex first.";
		LOG_IF ( set.isEmpty(), Debug, error )
				<< "Getting data from a empty image will result in undefined behavior.";
		LOG_IF ( !isInRange ( idx ), Debug, isis::error )
				<< "Index " << util::vector4<size_t> ( idx ) << " is out of range (" << getSizeAsString() << ")";
		const size_t index = getLinearIndex ( idx );
		return std::make_pair ( index / chunkVolume, index % chunkVolume );
	}


protected:
	bool clean;
	static const char *defaultChunkEqualitySet;

	/**
	 * Search for a dimensional break in all stored chunks.
	 * This function searches for two chunks whose (geometrical) distance is more than twice
	 * the distance between the first and the second chunk. It wll assume a dimensional break
	 * at this position.
	 *
	 * Normally chunks are beneath each other (like characters in a text) so their distance is
	 * more or less constant. But if there is a dimensional break (analogous to the linebreak
	 * in a text) the distance between this particular chunks/characters is bigger than twice
	 * the normal distance
	 *
	 * For example for an image of 2D-chunks (slices) getChunkStride(1) will
	 * get the number of slices (size of third dim) and  getChunkStride(slices)
	 * will get the number of timesteps
	 * \param base_stride the base_stride for the iteration between chunks (1 for the first
	 * dimension, one "line" for the second and soon...)
	 * \returns the length of this chunk-"line" / the stride
	 */
	size_t getChunkStride ( size_t base_stride = 1 );
	/**
	 * Access a chunk via index (and the lookup table)
	 * The Chunk will only have metadata which are unique to it - so it might be invalid
	 * (run join on it using the image as parameter to insert all non-unique-metadata).
	 */
	Chunk &chunkAt ( size_t at );
	/// Creates an empty Image object.
	Image();

public:
	/**
	 * Copy constructor.
	 * Copies all elements, only the voxel-data (in the chunks) are referenced.
	 */
	Image ( const Image &ref );

	/**
	 * Create image from a list of Chunks or objects with the base Chunk.
	 * Removes used chunks from the given list. So afterwards the list consists of the rejected chunks.
	 */
	template<typename T> Image ( std::list<T> &chunks, optional< util::slist& > rejected=optional< util::slist& >(), dimensions min_dim = rowDim ) :
		_internal::NDimensional<4>(), util::PropertyMap(), minIndexingDim ( min_dim ),
		set ( defaultChunkEqualitySet ),
		clean ( false ) {
		util::Singletons::get<NeededsList<Image>, 0>().applyTo( *this );
		set.addSecondarySort ( "acquisitionNumber" );
		insertChunksFromList ( chunks, rejected );
	}
	/**
	 * Create image from a vector of Chunks or objects with the base Chunk.
	 * Removes used chunks from the given list. So afterwards the list consists of the rejected chunks.
	 */
	template<typename T> Image ( std::vector<T> &chunks, optional< util::slist& > rejected=optional< util::slist& >(), dimensions min_dim = rowDim ) :
		_internal::NDimensional<4>(), util::PropertyMap(), minIndexingDim ( min_dim ),
		set ( defaultChunkEqualitySet ),
		clean ( false ) {
		util::Singletons::get<NeededsList<Image>, 0>().applyTo( *this );
		set.addSecondarySort ( "acquisitionNumber" );
		std::list<T> tmp( chunks.begin(), chunks.end() );
		insertChunksFromList ( tmp );
		chunks.assign( tmp.begin(), tmp.end() );
	}

	/**
	 * Insert Chunks or objects with the base Chunk from a sequence container into the Image.
	 * Removes used chunks from the given sequence container. So afterwards the container consists of the rejected chunks.
	 * \returns amount of successfully inserted chunks
	 */
	template<typename T> size_t insertChunksFromList ( std::list<T> &chunks, optional< util::slist& > rejected=optional< util::slist& >() ) {
		BOOST_MPL_ASSERT ( ( boost::is_base_of<Chunk, T> ) );
		size_t cnt = 0;

		for ( typename std::list<T>::iterator i = chunks.begin(); i != chunks.end(); ) { // for all remaining chunks
			if ( insertChunk ( *i ) ) {
				chunks.erase ( i++ );
				cnt++;
			} else {
				i++;
			}
		}

		if ( ! isEmpty() ) {
			LOG ( Debug, info ) << "Reindexing image with " << cnt << " chunks.";

			if ( !reIndex(rejected) ) {
				LOG ( Runtime, error ) << "Failed to create image from " << cnt << " chunks.";
			} else {
				LOG_IF ( !getMissing().empty(), Debug, warning )
						<< "The created image is missing some properties: " << util::MSubject( getMissing() ) << ". It will be invalid.";
			}
		} else {
			LOG ( Debug, warning ) << "Image is empty after inserting chunks.";
		}

		return cnt;
	}


	/**
	 * Create image from a single chunk.
	 */
	Image ( const Chunk &chunk, dimensions min_dim = rowDim );

	/**
	 * Copy operator.
	 * Copies all elements, only the voxel-data (in the chunks) are referenced.
	 */
	Image &operator= ( const Image &ref );

	bool checkMakeClean();
	bool isClean() const;
	/**
	 * This method returns a reference to the voxel value at the given coordinates.
	 *
	 * The voxel reference provides reading and writing access to the refered
	 * value.
	 *
	 * If the image is not clean, reIndex will be run.
	 * If the requested voxel is not of type T, an error will be raised.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position..
	 * \param second The second coordinate in voxel space. Usually the y value / the column-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the time-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * \returns A reference to the addressed voxel value. Reading and writing access
	 * is provided.
	 */
	template <typename T> T &voxel ( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 ) {
		checkMakeClean();
		const std::pair<size_t, size_t> index = commonGet ( first, second, third, fourth );
		ValueArray<T> &data = chunkAt ( index.first ).asValueArray<T>();
		return data[index.second];
	}

	void swapDim( unsigned short dim_a, unsigned short dim_b );

	/**
	 * Get a const reference to the voxel value at the given coordinates.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position..
	 * \param second The second coordinate in voxel space. Usually the y value / the column-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the time-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 *
	 * If the requested voxel is not of type T, an error will be raised.
	 *
	 * \returns A reference to the addressed voxel value. Only reading access is provided
	 */
	template <typename T> const T &voxel ( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0 ) const {
		const std::pair<size_t, size_t> index = commonGet ( first, second, third, fourth );
		const ValueArray<T> &data = chunkPtrAt ( index.first )->getValueArray<T>();
		return data[index.second];
	}

	const util::ValueReference getVoxelValue ( size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 ) const;
	void setVoxelValue ( const util::ValueReference &val, size_t nrOfColumns, size_t nrOfRows = 0, size_t nrOfSlices = 0, size_t nrOfTimesteps = 0 );

	/**
	 * Get the type of the chunk with "biggest" type.
	 * Determines the minimum and maximum of the image, (and with that the types of these limits).
	 * If they are not the same, the type which can store the other type is selected.
	 * E.g. if min is "-5(int8_t)" and max is "1000(int16_t)" "int16_t" is selected.
	 * Warning1: this will fail if min is "-5(int8_t)" and max is "70000(uint16_t)"
	 * Warning2: the cost of this is O(n) while Chunk::getTypeID is O(1) - so do not use it in loops
	 * Warning3: the result is not exact - so never use it to determine the type for Image::voxel (Use TypedImage to get an image with an guaranteed type)
	 * \returns a number which is equal to the ValueArray::staticID of the selected type.
	 */
	unsigned short getMajorTypeID() const;
	/// \returns the typename correspondig to the result of typeID
	std::string getMajorTypeName() const;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

	/**
	 * Get a chunk via index (and the lookup table).
	 * The returned chunk will be a cheap copy of the original chunk.
	 * If copy_metadata is true the metadata of the image is copied into the chunk.
	 */
	Chunk getChunkAt ( size_t at, bool copy_metadata = true ) const;

	/**
	 * Get the chunk that contains the voxel at the given coordinates.
	 *
	 * If the image is not clean, behaviour is undefined. (See Image::commonGet).
	 *
	 * \param first The first coordinate in voxel space. Usually the x value / the read-encoded position.
	 * \param second The second coordinate in voxel space. Usually the y value / the column-encoded position.
	 * \param third The third coordinate in voxel space. Ususally the z value / the slice-encoded position.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a copy of the chunk that contains the voxel at the given coordinates.
	 * (Reminder: Chunk-copies are cheap, so the image data are NOT copied but referenced)
	 */
	const Chunk getChunk ( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true ) const;

	/**
	 * Get the chunk that contains the voxel at the given coordinates.
	 * If the image is not clean Image::reIndex() will be run.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a copy of the chunk that contains the voxel at the given coordinates.
	 * (Reminder: Chunk-copies are cheap, so the image data are NOT copied but referenced)
	 */
	Chunk getChunk ( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true );

	/**
	 * Get the chunk that contains the voxel at the given coordinates in the given type.
	 * If the accordant chunk has type T a cheap copy is returned.
	 * Otherwise a MemChunk-copy of the requested type is created from it.
	 * In this case the minimum and maximum values of the image are computed and used for the MemChunk constructor.
	 *
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a (maybe converted) chunk containing the voxel value at the given coordinates.
	 */
	template<typename TYPE> Chunk getChunkAs ( size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true ) const {
		return getChunkAs<TYPE> ( getScalingTo ( ValueArray<TYPE>::staticID ), first, second, third, fourth, copy_metadata );
	}
	/**
	 * Get the chunk that contains the voxel at the given coordinates in the given type (fast version).
	 * \copydetails getChunkAs
	 * This version does not compute the scaling, and thus is much faster.
	 * \param scaling the scaling (scale and offset) to be used if a conversion to the requested type is neccessary.
	 * \param first The first coordinate in voxel space. Usually the x value.
	 * \param second The second coordinate in voxel space. Usually the y value.
	 * \param third The third coordinate in voxel space. Ususally the z value.
	 * \param fourth The fourth coordinate in voxel space. Usually the time value.
	 * \param copy_metadata if true the metadata of the image are merged into the returned chunk
	 * \returns a (maybe converted) chunk containing the voxel value at the given coordinates.
	 */
	template<typename TYPE> Chunk getChunkAs ( const scaling_pair &scaling, size_t first, size_t second = 0, size_t third = 0, size_t fourth = 0, bool copy_metadata = true ) const {
		Chunk ret = getChunk ( first, second, third, fourth, copy_metadata ); // get a cheap copy
		ret.convertToType ( ValueArray<TYPE>::staticID, scaling ); // make it of type T
		return ret; //return that
	}

	///for each chunk get the scaling (and offset) which would be used in an conversion to the given type
	scaling_pair getScalingTo ( unsigned short typeID, autoscaleOption scaleopt = autoscale ) const;


	/**
	 * Insert a Chunk into the Image.
	 * The insertion is sorted and unique. So the Chunk will be inserted behind a geometrically "lower" Chunk if there is one.
	 * If there is allready a Chunk at the proposed position this Chunk wont be inserted.
	 *
	 * \param chunk The Chunk to be inserted
	 * \returns true if the Chunk was inserted, false otherwise.
	 */
	bool insertChunk ( const Chunk &chunk );
	/**
	 * (Re)computes the image layout and metadata.
	 * The image will be "clean" on success.
	 * \returns true if the image was successfully reindexed and is valid, false otherwise.
	 */
	bool reIndex(optional< util::slist& > rejected=optional< util::slist& >());

	/// \returns true if there is no chunk in the image
	bool isEmpty() const;

	/**
	 * Get a list of the properties of the chunks for the given key.
	 * Retrieves the named property from each chunk and returnes them as a list.
	 * Not existing or empty properties will be inserted as empty properties. Thus the length of the list equals the amount of chunks.
	 * If unique is true those will be skipped. So the list might be shorter.
	 * \note Image properties (aka common properties) won't  be added
	 * \param key the name of the property to search for
	 * \param unique when true empty, non existing or consecutive duplicates won't be added
	 */
	std::list<util::PropertyValue> getChunksProperties ( const util::PropertyMap::key_type &key, bool unique = false ) const;

	/**
	 * Get a list of the properties of the chunks for the given key.
	 * Retrieves the named property from each chunk and returnes them as a list.
	 * Not existing or empty properties will be inserted as empty properties. Thus the length of the list equals the amount of chunks.
	 * If unique is true those will be skipped. So the list might be shorter.
	 * \note Image properties (aka common properties) won't  be added
	 * \param key the name of the property to search for
	 * \param unique when true empty, non existing or consecutive duplicates won't be added
	 */
	template<typename T> std::list<T> getChunksValuesAs ( const util::PropertyMap::key_type &key, bool unique = false ) const{
		std::list<T> ret;

		if( clean ) {
			BOOST_FOREACH( const boost::shared_ptr<Chunk> &ref, lookup ) {
				const optional< const util::PropertyValue& > prop = boost::const_pointer_cast<const Chunk>(ref)->hasProperty( key );

				if(unique){ // if unique
					if( ( prop && !ret.empty() &&  *prop == ret.back() ) || // if there is prop, skip if its equal
						!prop //if there is none skip anyway
					)
						continue;
				}
				ret.push_back( prop.get_value_or(util::PropertyValue(T())).as<T>() );
			}
		} else {
			LOG( Runtime, error ) << "Cannot get chunk-properties from non clean images. Run reIndex first";
		}

		return ret;
	}

	/**
	 * Get the size (in bytes) for the voxels in the image
	 * \warning As each Chunk of the image can have a different type (and thus bytesize),
	 * this does not neccesarly return the correct size for all voxels in the image. It
	 * will rather return the biggest size.
	 * \note The easiest (and most expensive) way to make sure you have the same bytesize accross the whole Image, is to run:
	 * \code
	 * image.convertToType(image.getMajorTypeID());
	 * \endcode
	 * assuming "image" is your image.
	 * \returns Get the biggest size (in bytes) accross all voxels in the image.
	 */
	size_t getMaxBytesPerVoxel() const;

	/**
	 * Get the maximum and the minimum voxel value of the image.
	 * The results are converted to T. If they dont fit an error ist send.
	 * \returns a pair of T storing the minimum and maximum values of the image.
	 */
	template<typename T> std::pair<T, T> getMinMaxAs() const {
		util::checkType<T>();// works only for T from _internal::types
		std::pair<util::ValueReference, util::ValueReference> minmax = getMinMax();
		return std::make_pair ( minmax.first->as<T>(), minmax.second->as<T>() );
	}

	/// Get the maximum and the minimum voxel value of the image as a pair of ValueReference-objects.
	std::pair<util::ValueReference, util::ValueReference> getMinMax() const;

	/**
	 * Compares the voxel-values of this image to the given.
	 * \returns the amount of the different voxels
	 */
	size_t compare ( const Image &comp ) const;

	orientation getMainOrientation() const;

	/**
	 * Transforms the image coordinate system into an other system by multiplying
	 * the orientation matrix with a user defined transformation matrix. Additionally,
	 * the index origin will be transformed into the new coordinate system. This
	 * function only changes the orientation information (rowVec, columnVec, sliceVec, indexOrigin)
	 * of the image but will not change the image itself.
	 *
	 * \warning If you call this function with a matrix other than the
	 * identidy matrix, it's not guaranteed that the image is still in ISIS space
	 * according to the DICOM conventions. Eventuelly some ISIS algorithms that
	 * depend on correct image orientations won't work as expected. Use this method
	 * with caution!
	 * \param transform_matrix the transformation matrix can be any type of rigid and affine transformation
	 * \param transformCenterIsImageCenter if this parameter is true, the center of the image will be translated to the
	 *  isocenter of the scanner prior applying the transform_matrix. Afterwards, it will be translated to its
	 *  initial position. For example this is the way SPM flips its images when converting from DICOM to nifti.
	 * \return returns if the transformation was successfuly
	 */
	bool transformCoords ( boost::numeric::ublas::matrix<float> transform_matrix, bool transformCenterIsImageCenter = false );

	/** Maps the given scanner Axis to the dimension with the minimal angle.
	 *  This is done by latching the orientation of the image by setting the biggest absolute
	 *  value of each orientation vector to 1 and the others to 0.
	 *  Example:
	 *          (-0.8)      (1)
	 *          ( 0.2)  ->  (0)   (this is done for the rowVec, columnVec and sliceVec)
	 *          (-0.1)      (0)
	 *
	 *  This latched orientation is used to map from the scanner axes to the dimension.
	 *  \param scannerAxes the axes of the scanner you want to map to dimension of the image.
	 *  \return the mapped image dimension
	 */

	dimensions mapScannerAxisToImageDimension ( scannerAxis scannerAxes );

	/**
	 * Copy all voxel data of the image into memory.
	 * If neccessary a conversion into T is done using min/max of the image.
	 * \param dst c-pointer for the memory to copy into
	 * \param len the allocated size of that memory in elements
	 * \param scaling the scaling to be used when converting the data (will be determined automatically if not given)
	 */
	template<typename T> void copyToMem ( T *dst, size_t len,  scaling_pair scaling = scaling_pair() ) const {
		if ( clean ) {
			if ( scaling.first.isEmpty() || scaling.second.isEmpty() ) {
				scaling = getScalingTo ( ValueArray<T>::staticID );
			}

			// we could do this using convertToType - but this solution does not need any additional temporary memory
			BOOST_FOREACH ( const boost::shared_ptr<Chunk> &ref, lookup ) {
				const size_t cSize = ref->getSizeAsVector().product();

				if ( !ref->copyToMem<T> ( dst, len, scaling ) ) {
					LOG ( Runtime, error ) << "Failed to copy raw data of type " << ref->getTypeName() << " from image into memory of type " << ValueArray<T>::staticName();
				} else {
					if ( len < cSize ) {
						LOG ( Runtime, error ) << "Abborting copy, because there is no space left in the target";
						break;
					}

					len -= cSize;
				}

				dst += ref->getVolume(); // increment the cursor
			}
		} else {
			LOG ( Runtime, error ) << "Cannot copy from non clean images. Run reIndex first";
		}
	}

	/**
	 * Copy all voxel data into a new MemChunk.
	 * This creates a MemChunk of the requested type and the same size as the Image and then copies all voxeldata of the image into that Chunk.
	 *
	 * If neccessary a conversion into T is done using min/max of the image.
	 *
	 * Also the properties of the first chunk are \link util::PropertyMap::join join\endlink-ed with those of the image and copied.
	 * \note This is a deep copy, no data will be shared between the Image and the MemChunk. It will waste a lot of memory, use it wisely.
	 * \returns a MemChunk containing the voxeldata and the properties of the Image
	 */
	template<typename T> MemChunk<T> copyAsMemChunk() const {
		const util::vector4<size_t> size = getSizeAsVector();
		data::MemChunk<T> ret ( size[0], size[1], size[2], size[3] );
		copyToMem<T> ( &ret.template voxel<T>( 0, 0 ), ret.getVolume() );
		static_cast<util::PropertyMap &>( ret ) = static_cast<const util::PropertyMap &>( getChunkAt( 0 ) );
		return ret;
	}

	/**
	 * Copy all voxel data into a new ValueArray.
	 * This creates a ValueArray of the requested type and the same length as the images volume and then copies all voxeldata of the image into that ValueArray.
	 *
	 * If neccessary a conversion into T is done using min/max of the image.
	 * \note This is a deep copy, no data will be shared between the Image and the ValueArray. It will waste a lot of memory, use it wisely.
	 * \returns a ValueArray containing the voxeldata of the Image (but not its Properties)
	 */
	template<typename T> ValueArray<T> copyAsValueArray() const {
		data::ValueArray<T> ret ( getVolume() );
		copyToMem<T> ( ret.begin().operator->(), ret.getLength() );
		return ret;
	}
	/**
	 * Copy all voxel data of the image into an existing ValueArray using its type.
	 * If neccessary a conversion into the datatype of the target is done using min/max of the image.
	 * \param dst ValueArray to copy into
	 * \param scaling the scaling to be used when converting the data (will be determined automatically if not given)
	 */
	void copyToValueArray ( data::ValueArrayBase &dst,  scaling_pair scaling = scaling_pair() ) const;

	/**
	 * Create a new Image of consisting of deep copied chunks.
	 * If neccessary a conversion into the requested type is done using the given scale.
	 * \param ID the ID of the requested type (type of the respective source chunk is used if not given)
	 * \param scaling the scaling to be used when converting the data (will be determined automatically if not given)
	 * \return a new deep copied Image of the same size
	 */
	Image copyByID( unsigned short ID = 0, scaling_pair scaling = scaling_pair() )const;


	/**
	* Get a sorted list of the chunks of the image.
	* \param copy_metadata set to false to prevent the metadata of the image to be copied into the results. This will improve performance, but the chunks may lack important properties.
	* \note These chunks will be cheap copies, so changing their voxels will change the voxels of the image. But you can for example use \code
	* std::vector< data::Chunk > cheapchunks=img.copyChunksToVector(); //this is a cheap copy
	* std::vector< data::MemChunk<float> > memchunks(cheapchunks.begin(),cheapchunks.end()); // this is not not
	* \endcode to get deep copies.
	*/
	std::vector<isis::data::Chunk> copyChunksToVector ( bool copy_metadata = true ) const;

	/**
	 * Ensure, the image has the type with the requested ID.
	 * If the typeID of any chunk is not equal to the requested ID, the data of the chunk is replaced by an converted version.
	 * The conversion is done using the value range of the image.
	 * \returns false if there was an error
	 */
	bool convertToType ( short unsigned int ID, isis::data::autoscaleOption scaleopt = autoscale );

	/**
	 * Automatically splice the given dimension and all dimensions above.
	 * e.g. spliceDownTo(sliceDim) will result in an image made of slices (aka 2d-chunks).
	 */
	size_t spliceDownTo ( dimensions dim );

	/**
	 * Run a functor with the base ChunkOp on every cunk in the image.
	 * This does not check the types of the images. So if your functor needs a specific type, use TypedImage.
	 * \param op a functor object which inherits ChunkOP
	 * \param copyMetaData if true the metadata of the image are copied into the chunks before calling the functor
	 */
	size_t foreachChunk ( ChunkOp &op, bool copyMetaData = false );


	/**
	 * Run a functor with the base VoxelOp on every chunk in the image.
	 * If any chunk does not have the requested type it will be converted.
	 * So the result is equivalent to TypedImage\<TYPE\>.
	 * If these conversion failes no operation is done, and false is returned.
	 * \param op a functor object which inherits ChunkOp
	 */
	template <typename TYPE> size_t foreachVoxel ( VoxelOp<TYPE> &op ) {
		class _proxy: public ChunkOp
		{
			VoxelOp<TYPE> &op;
		public:
			_proxy ( VoxelOp<TYPE> &_op ) : op ( _op ) {}
			bool operator() ( Chunk &ch, util::vector4<size_t> posInImage ) {
				return ch.foreachVoxel<TYPE> ( op, posInImage ) == 0;
			}
		};
		_proxy prx ( op );
		return convertToType ( data::ValueArray<TYPE>::staticID ) && foreachChunk ( prx, false );
	}

	/// \returns the number of rows of the image
	size_t getNrOfRows() const;
	/// \returns the number of columns of the image
	size_t getNrOfColumns() const;
	/// \returns the number of slices of the image
	size_t getNrOfSlices() const;
	/// \returns the number of timesteps of the image
	size_t getNrOfTimesteps() const;

	util::fvector3 getFoV() const;

	/**
	 * Generate a string identifying the image
	 * The identifier is made of
	 * - sequenceNumber
	 * - sequenceDescription if available
	 * - the common path of all chunk-sources (or the source file, if there is only one) if withpath is true
	 * - sequenceStart if available
	 * \param withpath add the common path of all sources to the identifying string
	 */
	std::string identify( bool withpath = true )const;
};

/**
 * An Image where all chunks are guaranteed to have a specific type.
 * This not necessarily means, that all chunks in this image are a deep copy of their origin.
 */
template<typename T> class TypedImage: public Image
{
protected:
	TypedImage() {} // to be used only by inheriting classes
public:
	typedef _internal::ImageIteratorTemplate<data::ValueArray<T> > iterator;
	typedef _internal::ImageIteratorTemplate<const data::ValueArray<T> > const_iterator;
	typedef typename iterator::reference reference;
	typedef typename const_iterator::reference const_reference;
	/// cheap copy another Image and make sure all chunks have type T
	TypedImage ( const Image &src ) : Image ( src ) { // ok we just copied the whole image
		//but we want it to be of type T
		convertToType ( ValueArray<T>::staticID );
	}
	/// cheap copy another TypedImage
	TypedImage &operator= ( const TypedImage &ref ) { //its already of the given type - so just copy it
		Image::operator= ( ref );
		return *this;
	}
	/// cheap copy another Image and make sure all chunks have type T
	TypedImage &operator= ( const Image &ref ) { // copy the image, and make sure its of the given type
		Image::operator= ( ref );
		convertToType ( ValueArray<T>::staticID );
		return *this;
	}
	void copyToMem ( void *dst ) {
		Image::copyToMem<T> ( ( T * ) dst );
	}
	void copyToMem ( void *dst ) const {
		Image::copyToMem<T> ( ( T * ) dst );
	}
	iterator begin() {
		if ( checkMakeClean() ) {
			boost::shared_array<data::ValueArray<T>*> vec( new data::ValueArray<T>*[lookup.size()] );

			for ( size_t i = 0; i < lookup.size(); i++ )
				vec[i] = &lookup[i]->template asValueArray<T>();

			return iterator ( vec, lookup.size() );
		} else {
			LOG ( Debug, error )  << "Image is not clean. Returning empty iterator ...";
			return iterator();
		}
	}
	iterator end() {
		return begin() + getVolume();
	};
	const_iterator begin() const {
		if ( isClean() ) {
			boost::shared_array<data::ValueArray<T>*> vec( new data::ValueArray<T>*[lookup.size()] );

			for ( size_t i = 0; i < lookup.size(); i++ )
				vec[i] = &lookup[i]->template asValueArray<T>();

			return const_iterator ( vec, lookup.size() );
		} else {
			LOG ( Debug, error )  << "Image is not clean. Returning empty iterator ...";
			return const_iterator();
		}
	}
	const_iterator end() const {
		return begin() + getVolume();
	};
};

/**
 * An Image which always uses its own memory and a specific type.
 * Thus, creating this image from another Image allways does a deep copy (and maybe a conversion).
 */
template<typename T> class MemImage: public TypedImage<T>
{
public:
	/**
	 * Copy contructor.
	 * This makes a deep copy of the given image.
	 * The image data are converted to T if necessary.
	 */
	MemImage ( const Image &src ) {
		operator= ( src );
	}

	/**
	 * Copy operator.
	 * This makes a deep copy of the given image.
	 * The image data are converted to T if necessary.
	 */
	MemImage &operator= ( const Image &ref ) { // copy the image, and make sure its of the given type

		Image::operator= ( ref ); // ok we just copied the whole image
		
		//we want deep copies of the chunks, and we want them to be of type T
		struct : _internal::SortedChunkList::chunkPtrOperator {
			std::pair<util::ValueReference, util::ValueReference> scale;
			boost::shared_ptr<Chunk> operator() ( const boost::shared_ptr< Chunk >& ptr ) {
				return boost::shared_ptr<Chunk> ( new MemChunk<T> ( *ptr, scale ) );
			}
		} conv_op;
		conv_op.scale = ref.getScalingTo ( ValueArray<T>::staticID );
		LOG ( Debug, info ) << "Computed scaling for conversion from source image: [" << conv_op.scale << "]";

		this->set.transform ( conv_op );

		if ( ref.isClean() ) {
			this->lookup = this->set.getLookup(); // the lookup table still points to the old chunks
		} else {
			LOG ( Debug, info ) << "Copied unclean image. Running reIndex on the copy.";
			this->reIndex();
		}

		return *this;
	}
};

}
}

#endif // IMAGE_H
