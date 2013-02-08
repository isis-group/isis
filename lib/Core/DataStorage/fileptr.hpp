//
//  fileptr.hpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef FILEPTR_HPP
#define FILEPTR_HPP

#define BOOST_FILESYSTEM_VERSION 3 
#include <boost/filesystem.hpp>
#include <boost/detail/endian.hpp>
#include "valuearray.hpp"
#include "endianess.hpp"

#ifdef WIN32
#include <windows.h>
#define FILE_HANDLE HANDLE
#else
#define FILE_HANDLE int
#endif

namespace isis
{
namespace data
{
/**
 * Class to map files into memory.
 * This can be used read only, or for read/write.
 *
 * Writing to a FilePtr mapping a file read-only is valid. It will not change the mapped file.
 *
 * This is inherting from ValueArray. Thus this, and all ValueArray created from it will be managed.
 * The mapped file will automatically unmapped and closed after all pointers a deleted.
 */
class FilePtr: public ValueArray<uint8_t>
{
	struct Closer {
		FILE_HANDLE file, mmaph;
		size_t len;
		boost::filesystem::path filename;
		bool write;
		void operator()( void *p );
	};
	typedef data::ValueArrayReference( *generator_type )( data::FilePtr &, size_t, size_t, bool );
	struct GeneratorMap: public std::map<unsigned short, generator_type> {
		GeneratorMap();
		template<class T> static data::ValueArrayReference generator( data::FilePtr &mfile, size_t offset, size_t len, bool swap_endianess ) {
			return mfile.at<T>( offset, len, swap_endianess );
		}
		struct proc {
			std::map<unsigned short, generator_type> *m_map;
			proc( std::map<unsigned short, generator_type> *map ): m_map( map ) {}
			template<class T> void operator()( const T & ) {
				m_map->insert( std::make_pair( ValueArray<T>::staticID, &generator<T> ) );
			}
		};
	};

	bool map( FILE_HANDLE file, size_t len, bool write, const boost::filesystem::path &filename );

	size_t checkSize( bool write, FILE_HANDLE file, const boost::filesystem::path &filename, size_t size = 0 );
	bool m_good, writing;
public:
	/// empty creator - result will not be usefull until filled
	FilePtr();
	/**
	 * Create a FilePtr, mapping the given file.
	 * if the write is true:
	 * - the file will be created if it does not exist
	 * - the file will be resized to len if it is shorter than len
	 * - a resize after this creation is NOT possible
	 *
	 * if write is false:
	 * - the length of the pointer will become the size of the file if len is 0 (so the whole file will be mapped)
	 * - if len is not 0 but less then the filesize, only len bytes of the file will be mapped
	 *
	 * creation will fail (good()!=true afterwards) if:
	 * - open/mmap fail on the given file for any reason
	 * - if file does not exist, write is true and len is 0
	 * - if write is false and len is greater then the length of the file or the file does not exist
	 *
	 * \param filename the file to map into memory
	 * \param len the requested length of the resulting ValueArray in bytes (automatically set if 0)
	 * \param write the file be opened for writing (writing to the mapped memory will write to the file, otherwise it will cause a copy-on-write)
	 */
	FilePtr( const boost::filesystem::path &filename, size_t len = 0, bool write = false );

	/**
	 * Get a ValueArray representing the data in the file.
	 * The resulting ValueArray will use a proxy deleter to keep track of the mapped file.
	 * So the file will be unmapped and closed if, and only if all ValueArray created by this function and the FilePtr are closed.
	 *
	 * If the FilePtr was opened writing, writing access to this ValueArray objects will result in writes to the file. Otherwise it will just write into memory.
	 *
	 * Note that there is no conversion done, just reinterpretation of the raw data in the file.
	 * \param offset the position in the file to start from (in bytes)
	 * \param len the requested length of the resulting ValueArray in elements (if that will go behind the end of the file, a warning will be issued).
	 * \param swap_endianess if endianess should be swapped when reading data file (ignored when used on files opened for writing)
	 */
	template<typename T> ValueArray<T> at( size_t offset, size_t len = 0, bool swap_endianess = false ) {
		boost::shared_ptr<T> ptr = boost::static_pointer_cast<T>( getRawAddress( offset ) );

		if( len == 0 ) {
			len = ( getLength() - offset ) / sizeof( T );
			LOG_IF( ( getLength() - offset ) % sizeof( T ), Runtime, warning )
					<< "The remaining filesize " << getLength() - offset << " does not fit the bytesize of the requested type "
					<< util::Value<T>::staticName();
		}

		LOG_IF( len * sizeof( T ) > ( getLength() - offset ), Debug, error )
				<< "The requested length will be " << len - ( getLength() - offset ) << " bytes behind the end of the file.";
		LOG_IF( writing && swap_endianess, Debug, warning )
				<< "Ignoring requested to swap byte order for writing (the systems byte order is " << BOOST_BYTE_ORDER << " and that will be used)";

		if( writing || !swap_endianess ) { // if not endianess swapping was requested or T is not float (or if we are writing)
			return data::ValueArray<T>( ptr, len ); // return a cheap copy
		} else { // flip bytes into a new ValueArray
			LOG( Debug, info ) << "Byte swapping " <<  ValueArray<T>::staticName() << " for endianess";
			ValueArray<T> ret( len );
			data::endianSwapArray( ptr.get(), ptr.get() + std::min( len, getLength() / sizeof( T ) ), ret.begin() );
			return ret;
		}

	}

	/**
	 * Get a ValueArrayReference to a ValueArray of the requested type.
	 * The resulting ValueArray will use a proxy deleter to keep track of the mapped file.
	 * So the file will be unmapped and closed if, and only if all ValueArray created by this function and the FilePtr are closed.
	 *
	 * If the FilePtr was opened writing, writing access to this ValueArray objects will result in writes to the file.
	 * Otherwise it will just write into memory.
	 *
	 * If the FilePtr was opened reading and the assumed endianess of the file (see parameter) does not fit the endianess
	 * of the system an (endianess-converted) deep copy is created.
	 *
	 * \param ID the requested type (note that there is no conversion done, just reinterpretation of the raw data in the file)
	 * \param offset the position in the file to start from (in bytes)
	 * \param len the requested length of the resulting ValueArray in elements (if that will go behind the end of the file, a warning will be issued).
	 * \param swap_endianess if endianess should be swapped when reading data file (ignored when used on files opened for writing)
	 */
	data::ValueArrayReference atByID( unsigned short ID, size_t offset, size_t len = 0, bool swap_endianess = false );

	bool good();
	void release();
};

}
}

#endif
