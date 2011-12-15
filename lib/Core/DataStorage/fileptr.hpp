//
//  fileptr.hpp
//  isis
//
//  Created by Enrico Reimer on 08.08.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef FILEPTR_HPP
#define FILEPTR_HPP

#define BOOST_FILESYSTEM_VERSION 2 //@todo remove as soon as we switch to boost 1.42
#include <boost/filesystem.hpp>
#include "typeptr.hpp"

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
 * This is inherting from ValuePtr. Thus this, and all ValuePtr created from it will be managed.
 * The mapped file will automatically unmapped and closed after all pointers a deleted.
 */
class FilePtr: public ValuePtr<uint8_t>
{
	struct Closer {
		int file;
		size_t len;
		boost::filesystem::path filename;
		bool write;
		void operator()( void *p );
	};
	typedef data::ValuePtrReference( *generator_type )( data::FilePtr &, size_t, size_t );
	struct GeneratorMap: public std::map<unsigned short, generator_type> {
		GeneratorMap();
		template<class T> static data::ValuePtrReference generator( data::FilePtr &mfile, size_t offset, size_t len ) {return mfile.at<T>( offset, len );}
		struct proc {
			std::map<unsigned short, generator_type> *m_map;
			proc( std::map<unsigned short, generator_type> *map ): m_map( map ) {}
			template<class T> void operator()( const T & ) {
				m_map->insert( std::make_pair( ValuePtr<T>::staticID, &generator<T> ) );
			}
		};
	};

	bool map( int file, size_t len, bool write, const boost::filesystem::path &filename );
	size_t checkSize( bool write, int file, const boost::filesystem::path &filename, size_t size = 0 );
	bool m_good;
public:
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
	 * \param len the requested length of the resulting ValuePtr in bytes (automatically set if 0)
	 * \param write the file be opened for writing (writing to the mapped memory will write to the file, otherwise it will cause a copy-on-write)
	 */
	FilePtr( const boost::filesystem::path &filename, size_t len = 0, bool write = false );

	/**
	 * Get a ValuePtr representing the data in the file.
	 * The resulting ValuePtr will use a proxy deleter to keep track of the mapped file.
	 * So the file will be unmapped and closed if, and only if all ValuePtr created by this function and the FilePtr are closed.
	 *
	 * If the FilePtr was opened writing, writing access to this ValuePtr objects will result in writes to the file. Otherwise it will just write into memory.
	 *
	 * Note that there is no conversion done, just reinterpretation of the raw data in the file.
	 * \param offset the position in the file to start from (in bytes)
	 * \param len the requested length of the resulting ValuePtr in elements (if that will go behind the end of the file, a warning will be issued).
	 */
	template<typename T> ValuePtr<T> at( size_t offset, size_t len = 0 ) {
		boost::shared_ptr<T> ptr = boost::static_pointer_cast<T>( getRawAddress( offset ) );

		if( len == 0 ) {
			len = ( getLength() - offset ) / sizeof( T );
			LOG_IF( ( getLength() - offset ) % sizeof( T ), Runtime, warning )
					<< "The remaining filesize " << getLength() - offset << " does not fit the bytesize of the requested type "
					<< util::Value<T>::staticName();
		}

		LOG_IF( len * sizeof( T ) > ( getLength() - offset ), Debug, error ) << "The requested length will be " << len - ( getLength() - offset ) << " bytes behind the end of the source.";
		return data::ValuePtr<T>( ptr, len );
	}

	/**
	 * Get a ValuePtrReference to a ValuePtr of the requested type.
	 * The resulting ValuePtr will use a proxy deleter to keep track of the mapped file.
	 * So the file will be unmapped and closed if, and only if all ValuePtr created by this function and the FilePtr are closed.
	 *
	 * If the FilePtr was opened writing, writing access to this ValuePtr objects will result in writes to the file. Otherwise it will just write into memory.
	 * \param ID the requested type (note that there is no conversion done, just reinterpretation of the raw data in the file)
	 * \param offset the position in the file to start from (in bytes)
	 * \param len the requested length of the resulting ValuePtr in elements (if that will go behind the end of the file, a warning will be issued).
	 */
	data::ValuePtrReference atByID( unsigned short ID, size_t offset, size_t len = 0 );

	bool good();
	void close();
};

}
}

#endif
